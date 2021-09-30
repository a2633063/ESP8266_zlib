#ifndef __ESP8266_ZLIB_JSON_H__
#define __ESP8266_ZLIB_JSON_H__

#include "osapi.h"

#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "zlib.h"
#include "../cJson/cJSON.h"



/**
 * 函  数  名: zlib_json_deal_callback_function
 * 函数说明: zlib库json回调函数定义
 * 参        数: arg:通信相关指针
 *          type:通信类型
 *          pJsonRoot:需要处理的json字符串
 *          p:通信附带的其他数据   mqtt:topic字符串     http:get数据(可能为空字符串)
 * 返        回: 无
 */
typedef void (*zlib_json_deal_callback_function)(void *arg, Wifi_Comm_type_t type, cJSON * pJsonRoot, void *p) ;


extern void zlib_json_init(zlib_json_deal_callback_function cb);
extern void zlib_json_deal(void *arg, Wifi_Comm_type_t type, char* jsonRoot, void *p);


#endif

