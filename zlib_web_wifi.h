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

#define ZLIB_WEB_WIFI_HTTP \
 { "/", web_send_wifisetting_page, NULL, NULL, NULL }, \
 { "/result.htm", NULL,web_send_result_page, NULL, NULL },  \
 { "/setting.htm", web_send_wifisetting_page, NULL, NULL, NULL }


extern int ICACHE_FLASH_ATTR web_send_wifisetting_page(void *arg, URL_Frame *purl_frame);
extern int ICACHE_FLASH_ATTR web_send_result_page(void *arg, URL_Frame *purl_frame);

#endif

