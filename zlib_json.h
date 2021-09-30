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




//回调函数,tcp接收到数据后先调用此回调函数
//返回值: true: 继续执行json处理	false:不执行json处理
typedef bool (*zlib_json_deal_callback_function)(void *arg, Wifi_Comm_type_t type, cJSON * pJsonRoot) ;


extern void zlib_json_init(zlib_json_deal_callback_function cb);
extern void zlib_json_deal(void *arg, Wifi_Comm_type_t type, char* jsonRoot);


#endif

