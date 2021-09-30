#ifndef __ESP8266_ZLIB_H__
#define __ESP8266_ZLIB_H__

#include "osapi.h"

#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
//#include "zlib.h"
#include "../cJson/cJSON.h"
#include "zlib_wifi.h"
#include "zlib_web_server.h"
#include "zlib_web_wifi.h"
#include "zlib_function.h"
#include "zlib_udp.h"
#include "zlib_tcp.h"


#ifndef DEVICE_NAME	//长度不可超过31
	#define DEVICE_NAME "zControl_%s"
#endif

typedef enum {
    WIFI_COMM_TYPE_UDP = 0,
    WIFI_COMM_TYPE_TCP,
    WIFI_COMM_TYPE_HTTP,
    WIFI_COMM_TYPE_MQTT,
    WIFI_COMM_TYPE__MAX
} Wifi_Comm_type_t;

#include "zlib_json.h"
#if (1)	//调试输出相关
//LOGD	0
//LOGI	1
//LOGW	2
//LOGE	3
#define DEBUG_LEVEL	0	//0显示全部log 4不输出任何log


#if (DEBUG_LEVEL < 1)
	#define LOGD( format, ... )    os_printf( format, ## __VA_ARGS__ )
#else
	#define LOGD( format, ... )
#endif

#if (DEBUG_LEVEL < 2)
	#define LOGI( format, ... )    os_printf( format, ## __VA_ARGS__ )
#else
	#define LOGI( format, ... )
#endif

#if (DEBUG_LEVEL < 3)
	#define LOGW( format, ... )    os_printf( format, ## __VA_ARGS__ )
#else
	#define LOGW( format, ... )
#endif

#if (DEBUG_LEVEL < 4)
	#define LOGE( format, ... )    os_printf( format, ## __VA_ARGS__ )
#else
	#define LOGE( format, ... )
#endif

#endif



#endif
