#include "stdlib.h"
#include "osapi.h"
#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "zlib.h"
#include "zlib_setting.h"
#include "zlib_web_server.h"
#include "zlib_web_config.h"
#include "zlib_function.h"

//------------------    配置wifi部分    ------------------------------------------------------
/**
 * 函  数  名: zlib_web_config_send_wifisetting_page
 * 函数说明: web配网功能 http回调反馈函数
 * 参        数:
 * 返        回: 无
 */
int ICACHE_FLASH_ATTR zlib_web_config_send_wifisetting_page(void *arg, URL_Frame *purl_frame)
{
    struct espconn *ptrespconn = arg;
    zlib_web_server_reply(ptrespconn, true, TEXT_HTML, (char *) web_wifisetting_html);
    return 0;
}
/**
 * 函  数  名: zlib_web_config_json_get/zlib_web_config_json_post
 * 函数说明: web http回复get/post json
 * 参        数:
 * 返        回: 无
 */
int ICACHE_FLASH_ATTR zlib_web_config_json_get(void *arg, URL_Frame *purl_frame)
{
    struct espconn *ptrespconn = arg;
    char temp_str[URL_GET_DAT_MAX_LENGTH];
    zlib_web_server_decode(purl_frame->pGetdat, temp_str, URL_GET_DAT_MAX_LENGTH);
    LOGD("[ZLIB_WEB_CONFIG]json get:%s\n",temp_str);
    zlib_json_deal(arg, WIFI_COMM_TYPE_HTTP, temp_str, purl_frame->pGetdat);
    return 0;
}
int ICACHE_FLASH_ATTR zlib_web_config_json_post(void *arg, URL_Frame *purl_frame)
{
    struct espconn *ptrespconn = arg;
    //zlib_web_server_reply(ptrespconn, true, APPLICATIOIN_JSON, (char *)web_wifisetting_html);
    zlib_json_deal(arg, WIFI_COMM_TYPE_HTTP, purl_frame->pPostdat, purl_frame->pGetdat);
    return 0;
}
/**
 * 函  数  名: zlib_web_config_send_result_page
 * 函数说明: web配网功能 http回调反馈函数
 * 参        数:
 * 返        回: 无
 */
int ICACHE_FLASH_ATTR zlib_web_config_send_result_page(void *arg, URL_Frame *purl_frame)
{
    struct espconn *ptrespconn = arg;

    char *str = NULL;
    int16_t length = 0;

    char ssid[32] = { 0 };
    char password[64] = { 0 };

    os_memset(ssid, 0, 32);
    os_memset(password, 0, 64);

    if(purl_frame == NULL)
    {
        goto Error;
    }

    length = zlib_web_server_get_tag_val(purl_frame->pPostdat, "SSID", ssid, 32);
    LOGI("[ZLIB_WEB_CONFIG]ssid[%d]:%s\n", length, ssid);
    if(os_strlen(ssid) < 1) goto Error;

    length = zlib_web_server_get_tag_val(purl_frame->pPostdat, "PASS", password, 64);
    LOGI("[ZLIB_WEB_CONFIG]password[%d]:%s\n", length, password);
    if(os_strlen(password) > 0 && os_strlen(password) < 8) goto Error;
    zlib_web_server_reply(ptrespconn, true, TEXT_HTML, (char *) web_wifisuccess_html);

    //连接wifi
    zlib_wifi_set_ssid_delay(ssid, password, 1000);
//    wifi_station_disconnect();//不可直接连接 需要延时回复后才可以连接
//    wifi_station_connect();
    zlib_reboot_delay(2000);
    return 0;
    Error: zlib_web_server_reply(ptrespconn, true, TEXT_HTML, (char *) web_wififail_html);
    return -1;
}
//------------------    wifi部分结束    ------------------------------------------------------

//------------------    配置mqtt部分    ------------------------------------------------------
/**
 * 函  数  名: zlib_web_mqtt_send_wifisetting_page
 * 函数说明: web配网功能 http回调反馈函数
 * 参        数:
 * 返        回: 无
 */
int ICACHE_FLASH_ATTR zlib_web_mqtt_send_wifisetting_page(void *arg, URL_Frame *purl_frame)
{
    struct espconn *ptrespconn = arg;
    zlib_web_server_reply(ptrespconn, true, TEXT_HTML, (char *) web_mqttsetting_html);
    return 0;
}

/**
 * 函  数  名: zlib_web_mqtt_send_result_page
 * 函数说明: web配网功能 http回调反馈函数
 * 参        数:
 * 返        回: 无
 */
int ICACHE_FLASH_ATTR zlib_web_mqtt_send_result_page(void *arg, URL_Frame *purl_frame)
{
    struct espconn *ptrespconn = arg;

    int16_t length = 0;

    char url[64] = { 0 };
    char user[64] = { 0 };
    char password[64] = { 0 };

    os_memset(url, 0, 64);
    os_memset(user, 0, 64);
    os_memset(password, 0, 64);

    if(purl_frame == NULL)
    {
        goto Error;
    }

    length = zlib_web_server_get_tag_val(purl_frame->pPostdat, "USER", user, 64);
    LOGI("[ZLIB_WEB_CONFIG]USER[%d]:%s\n", length, user);
    length = zlib_web_server_get_tag_val(purl_frame->pPostdat, "PASS", password, 64);
    LOGI("[ZLIB_WEB_CONFIG]PASS[%d]:%s\n", length, password);
    length = zlib_web_server_get_tag_val(purl_frame->pPostdat, "URL", url, 64);
    LOGI("[ZLIB_WEB_CONFIG]URL[%d]:%s\n", length, url);

    char *p = os_strstr(url, ":");
    if(p == NULL)
    {
        user_config.mqtt_port = 1883;
    }
    else
    {
        if(p - url < 3) goto Error;

        uint32_t val = 0;
        *p = '\0';
        p++;
        while (*p != '\0')
        {
            if(*p >= '0' && *p <= '9')
            {
                val = val * 10 + *p - '0';
                p++;
            }
            else
                goto Error;
        }
        if(val > 0 && val < 65536)
        {
            user_config.mqtt_port = val;
        }
        else
            goto Error;
    }
    os_strcpy(user_config.mqtt_ip, url);
    os_strcpy(user_config.mqtt_user, user);
    os_strcpy(user_config.mqtt_password, password);

    LOGI("[ZLIB_WEB_CONFIG]url:%s:%d\n", url, user_config.mqtt_port);
    LOGI("[ZLIB_WEB_CONFIG]user:%s\n", user);
    LOGI("[ZLIB_WEB_CONFIG]password:%s\n", password);

    zlib_web_server_reply(ptrespconn, true, TEXT_HTML, (char *) web_mqttsuccess_html);

    //配置mqtt

    zlib_setting_save_config(&user_config, sizeof(user_config_t));
    if(zlib_wifi_get_state() == STATE_WIFI_STAMODE_GOT_IP) zlib_reboot_delay(2000);
    return 0;
    Error: zlib_web_server_reply(ptrespconn, true, TEXT_HTML, (char *) web_mqttfail_html);
    return -1;
}
//------------------    mqtt部分结束    ------------------------------------------------------

#if (ZLIB_WEB_CONFIG_ONLY)
const static URL_Http_Call_t g_app_handlers[] = {
ZLIB_WEB_CONFIG_HTTP };
/**
 * 函  数  名: zlib_web_config_init
 * 函数说明: web服务仅web配网功能时调用
 * 参        数:
 * 返        回: 无
 */
void ICACHE_FLASH_ATTR zlib_web_config_init(void)
{
    zlib_web_server_init(80, g_app_handlers, sizeof(g_app_handlers) / sizeof(URL_Http_Call_t));
    LOGI("[ZLIB_WEB_CONFIG]web for wifi config Init!\n");
}
#endif
