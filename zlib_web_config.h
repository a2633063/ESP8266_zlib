#ifndef __ESP8266_ZLIB_WEB_CONFIG_H__
#define __ESP8266_ZLIB_WEB_CONFIG_H__

#include "osapi.h"

#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "zlib.h"
#include "zlib_web_server.h"

#include "zlib_web_config_web_data.h"

#ifndef ZLIB_WEB_CONFIG_ONLY
#define ZLIB_WEB_CONFIG_ONLY  (0)  //若使用了自定义webserver 此应该设置为0 若仅用web配网 此置1 且调用zlib_web_config_init即可
#endif

#define ZLIB_WEB_CONFIG_HTTP \
 { "/", zlib_web_config_send_wifisetting_page, NULL, NULL, NULL }, \
 { "/result", NULL,zlib_web_config_send_result_page, NULL, NULL },  \
 { "/json", zlib_web_config_json_get, zlib_web_config_json_post, NULL, NULL },        \
 { "/mqtt", zlib_web_mqtt_send_wifisetting_page, NULL, NULL, NULL }, \
 { "/mqttresult", NULL,zlib_web_mqtt_send_result_page, NULL, NULL }

#if (ZLIB_WEB_CONFIG_ONLY)
extern void zlib_web_config_init(void);
#endif
extern int ICACHE_FLASH_ATTR zlib_web_config_send_wifisetting_page(void *arg, URL_Frame *purl_frame);
extern int ICACHE_FLASH_ATTR zlib_web_config_send_result_page(void *arg, URL_Frame *purl_frame);
extern int ICACHE_FLASH_ATTR zlib_web_config_json_post(void *arg, URL_Frame *purl_frame);
extern int ICACHE_FLASH_ATTR zlib_web_config_json_get(void *arg, URL_Frame *purl_frame);
extern int ICACHE_FLASH_ATTR zlib_web_mqtt_send_result_page(void *arg, URL_Frame *purl_frame);
extern int ICACHE_FLASH_ATTR zlib_web_mqtt_send_topic_page(void *arg, URL_Frame *purl_frame);

#endif

