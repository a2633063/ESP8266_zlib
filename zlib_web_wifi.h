#ifndef __ESP8266_ZLIB_WEB_WIFI_H__
#define __ESP8266_ZLIB_WEB_WIFI_H__

#include "osapi.h"

#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "zlib.h"
#include "zlib_web_server.h"

#include "zlib_web_wifi_web_data.h"


#ifndef ZLIB_WEB_WIFI_ONLY
#define ZLIB_WEB_WIFI_ONLY  (0)  //若使用了自定义webserver 此应该设置为0 若仅用web配网 此置1 且调用zlib_web_wifi_init即可
#endif

#define ZLIB_WEB_WIFI_HTTP \
 { "/", zlib_web_wifi_send_wifisetting_page, NULL, NULL, NULL }, \
 { "/result.htm", NULL,zlib_web_wifi_send_result_page, NULL, NULL },  \
 { "/setting.htm", zlib_web_wifi_send_wifisetting_page, NULL, NULL, NULL }


#if (ZLIB_WEB_WIFI_ONLY)
extern void zlib_web_wifi_init(void);
#endif
extern int ICACHE_FLASH_ATTR zlib_web_wifi_send_wifisetting_page(void *arg, URL_Frame *purl_frame);
extern int ICACHE_FLASH_ATTR zlib_web_wifi_send_result_page(void *arg, URL_Frame *purl_frame);

#endif

