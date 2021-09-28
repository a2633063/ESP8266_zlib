#include "stdlib.h"
#include "osapi.h"
#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "zlib.h"
#include "zlib_web_server.h"
#include "zlib_web_wifi.h"
#include "zlib_function.h"


/**
 * 函  数  名: zlib_web_wifi_send_wifisetting_page
 * 函数说明: web配网功能 http回调反馈函数
 * 参        数:
 * 返        回: 无
 */
int ICACHE_FLASH_ATTR zlib_web_wifi_send_wifisetting_page(void *arg, URL_Frame *purl_frame)
{
    struct espconn *ptrespconn = arg;
    zlib_web_server_reply(ptrespconn, true, TEXT_HTML, (char *)web_wifisetting_html);
    return 0;
}
/**
 * 函  数  名: zlib_web_wifi_send_result_page
 * 函数说明: web配网功能 http回调反馈函数
 * 参        数:
 * 返        回: 无
 */
int ICACHE_FLASH_ATTR zlib_web_wifi_send_result_page(void *arg, URL_Frame *purl_frame)
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
    LOGI("[ZLIB_WEB_WIFI]ssid[%d]:%s\n", length, ssid);
    if(os_strlen(ssid) < 1) goto Error;

    length = zlib_web_server_get_tag_val(purl_frame->pPostdat, "PASS", password, 64);
    LOGI("[ZLIB_WEB_WIFI]password[%d]:%s\n", length, password);
    if(os_strlen(password) > 0 && os_strlen(password) < 8) goto Error;
    zlib_web_server_reply(ptrespconn, true, TEXT_HTML, (char *)web_wifisuccess_html);

    //连接wifi
    zlib_wifi_set_ssid(ssid, password);
//    wifi_station_disconnect();//不可直接连接 需要延时回复后才可以连接
//    wifi_station_connect();
    zlib_reboot_delay(1000);
    return 0;
    Error: zlib_web_server_reply(ptrespconn, true, TEXT_HTML,(char *) web_wififail_html);
    return -1;
}


#if (ZLIB_WEB_WIFI_ONLY)
static URL_Http_Call_t g_app_handlers[] = {
        ZLIB_WEB_WIFI_HTTP
};
/**
 * 函  数  名: zlib_web_wifi_init
 * 函数说明: web服务仅web配网功能时调用
 * 参        数:
 * 返        回: 无
 */
void ICACHE_FLASH_ATTR zlib_web_wifi_init(void)
{
    zlib_web_server_init(80,g_app_handlers,sizeof(g_app_handlers) / sizeof(URL_Http_Call_t));
    LOGI("[ZLIB_WEB_WIFI]web for wifi config Init!\n");
}
#endif
