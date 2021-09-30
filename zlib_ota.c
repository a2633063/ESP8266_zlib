#include "zlib.h"
#include "upgrade.h"
#include "zlib_ota.h"

#include "utils.h"

static struct upgrade_server_info ota_server;
static zlib_ota_result_callback_function _ota_callback = NULL;
void ICACHE_FLASH_ATTR _ota_finished_callback(void *arg)
{

    char str[40];
    struct upgrade_server_info *update = arg;
    if(update->upgrade_flag == 1)
    {
        LOGI("[ZLIB_OTA]ota success; rebooting!\n");

        if(_ota_callback != NULL) _ota_callback(STATE_OTA_RESULT_SUCCESS);
        //user_wifi_send(NULL, WIFI_COMM_TYPE_MQTT, "", false);
        zlib_reboot_delay(1500);
    }
    else
    {
        if(_ota_callback != NULL) _ota_callback(STATE_OTA_RESULT_FAIL);
        LOGI("[ZLIB_OTA]failed!\n");
    }

    if(ota_server.pespconn != NULL) os_free(ota_server.pespconn);
    if(ota_server.url != NULL) os_free(ota_server.url);
    //os_free(update);

}
static void ICACHE_FLASH_ATTR _zlib_ota_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
    //struct espconn *pespconn = (struct espconn *) arg;

    if(ipaddr == NULL)
    {
        LOGE("[ZLIB_OTA]dns found fail:%s", name);
        if(_ota_callback != NULL) _ota_callback(STATE_OTA_RESULT_DNS_FAIL);
        return;
    }
    LOGD("[ZLIB_OTA]dns found: [%s]:[%d.%d.%d.%d]\n", name, *((uint8 * ) &ipaddr->addr),
            *((uint8 * ) &ipaddr->addr + 1), *((uint8 * ) &ipaddr->addr + 2), *((uint8 * ) &ipaddr->addr + 3));

    os_memcpy(ota_server.ip, &ipaddr->addr, 4);
    ota_server.check_cb = _ota_finished_callback;
    ota_server.check_times = 10000;
    if(ota_server.pespconn == NULL) ota_server.pespconn = (struct espconn *) os_zalloc(sizeof(struct espconn));

    if(ota_server.pespconn == NULL)
    {
        LOGE("[ZLIB_OTA]Error:os_zalloc for pespconn fail!\n");
        if(ota_server.pespconn != NULL) os_free(ota_server.pespconn);
        if(ota_server.url != NULL) os_free(ota_server.url);
        if(_ota_callback != NULL) _ota_callback(STATE_OTA_RESULT_MEM_FAIL);
    }
    if(system_upgrade_start(&ota_server) == false)
    {
        LOGE("[ZLIB_OTA]Error:could not start ota!\n");
        if(ota_server.pespconn != NULL) os_free(ota_server.pespconn);
        if(ota_server.url != NULL) os_free(ota_server.url);
        //os_free(update);
        if(_ota_callback != NULL) _ota_callback(STATE_OTA_RESULT_FAIL);
    }
    else
    {
        LOGI("[ZLIB_OTA]OTA...\n");
    }

    /*
     if(ipaddr != NULL)
     {
     os_printf("user_esp_platform_dns_found %d.%d.%d.%d\n", *((uint8 *) &ipaddr->addr),
     *((uint8 *) &ipaddr->addr + 1), *((uint8 *) &ipaddr->addr + 2), *((uint8 *) &ipaddr->addr + 3));

     if(ota_path == NULL)
     {
     os_printf("OTA fail:path is null!\n");
     return;
     }

     ota_start_Upgrade(name, (char *) &ipaddr->addr);
     }
     else
     {
     os_printf("user_esp_platform_dns_found fail\n");
     }*/
}
/**
 * 函  数  名: zlib_ota_start
 * 函数说明: 开始ota
 * 参        数: s:链接地址
 * 返        回: 0:无错误    -1链接地址格数错误
 */
int8_t ICACHE_FLASH_ATTR zlib_ota_start(char *s)
{
    int8_t r = 0;
    //分解域名/端口/链接地址

    char *p;
    char * domain = NULL;
    char *domain_p;
    char *path = NULL;
    char *path_p;

    uint16_t max_length = os_strlen(s);

    p = s;
    if(domain != NULL) os_free(domain);
    domain = (char *) os_malloc(max_length);
    os_memset(domain, 0, max_length);
    domain_p = domain;

    //查找域名开始位置
    if(os_strncmp(p, "https://", 8) == 0)
    {
        p += 8;
    }
    else if(os_strncmp(p, "http://", 7) == 0)
    {
        p += 7;
    }
    else
    {
        r = -1;
        goto ERROR;
        //return -1;
    }

    ota_server.port = 0;

    //获取域名/ip:端口
    while (*p != '\0')
    {
        if(*p == '/')
        {
            *domain_p = '\0';
            break;
        }
        else if(*p == ':')
        {
            *domain_p = '\0';
            p++;
            while (*p >= '0' && *p <= '9')
            {
                ota_server.port = ota_server.port * 10 + *p - '0';
                p++;
            }
            break;
        }
        else
        {
            *domain_p = *p;
            p++;
            domain_p++;
        }
    }
    if(ota_server.port == 0) ota_server.port = 80;

    //获取链接后续内容
    if(path != NULL) os_free(path);
    path = (char *) os_malloc(max_length);
    os_memset(path, 0, max_length);
    path_p = path;
    while (*p != '\0')
    {
        *path_p = *p;
        p++;
        path_p++;
    }

    //合成http Get请求字符串
    if(ota_server.url == NULL) ota_server.url = (uint8 *) os_zalloc(512);
    if(ota_server.url == NULL)
    {
        r = -2;
        goto ERROR;
    }
    os_sprintf((char* ) ota_server.url, "GET %s HTTP/1.1\r\n"
            "Host: %s:%d\r\n"
            "Connection: keep-alive\r\n"
            "\r\n", path, domain, ota_server.port);
    LOGD("[ZLIB_OTA]http: \n%s\n", ota_server.url);

    struct espconn socket;
    ip_addr_t ip_addr;

    if(UTILS_StrToIP(domain, &(ip_addr.addr)))
    {
        os_printf("domain is ip:%s\r\n", domain);
        _zlib_ota_dns_found(domain, &ip_addr, &socket);
        //ota_start_Upgrade(domain, (char *) &(ip_addr.addr));
    }
    else
    {
        os_printf("get domain [%s] ip....\r\n", domain);
        espconn_gethostbyname(&socket, domain, &ip_addr, _zlib_ota_dns_found);
    }
    r = 0;
    ERROR: if(path != NULL) os_free(path);
    if(domain != NULL) os_free(domain);
    if(r < 0)
    {
        if(ota_server.url != NULL) os_free(ota_server.url);
    }
    return r;
}

/**
 * 函  数  名: zlib_ota_set_result_callback
 * 函数说明: 设置ota结果
 * 参        数: s:链接地址
 * 返        回: 0:无错误    -1链接地址格数错误
 */
int8_t ICACHE_FLASH_ATTR zlib_ota_set_result_callback(zlib_ota_result_callback_function cb)
{
    _ota_callback = cb;
}
