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

//URL_Http_Call_t g_app_handlers[] = { { "/", web_send_wifisetting_page, NULL, NULL, NULL }, { "/result.htm", NULL,
//        web_send_result_page, NULL, NULL }, { "/setting.htm", web_send_wifisetting_page, NULL, NULL, NULL } };

int ICACHE_FLASH_ATTR web_send_wifisetting_page(void *arg, URL_Frame *purl_frame)
{
    struct espconn *ptrespconn = arg;
    zlib_web_server_reply(ptrespconn, true, TEXT_HTML, web_wifisetting_html);
    return 0;
}

int ICACHE_FLASH_ATTR web_send_result_page(void *arg, URL_Frame *purl_frame)
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

    length = zlib_web_server_get_tag_val(purl_frame->pPostdat, "SSID@", ssid, 32);
    LOGI("[ZLIB_WEB_WIFI]ssid[%d]:%s\n", length, ssid);
    if(os_strlen(ssid) < 1) goto Error;

    length = zlib_web_server_get_tag_val(purl_frame->pPostdat, "PASS", password, 64);
    LOGI("[ZLIB_WEB_WIFI]password[%d]:%s\n", length, password);
    if(os_strlen(password) > 0 && os_strlen(password) < 8) goto Error;
    zlib_web_server_reply(ptrespconn, true, TEXT_HTML, web_wifisuccess_html);

    //连接wifi
    //user_wifi_set(ssid, password);
    return 0;
    Error: zlib_web_server_reply(ptrespconn, true, TEXT_HTML, web_wififail_html);
    return -1;
}
