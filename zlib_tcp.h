#ifndef __ESP8266_ZLIB_TCP_H__
#define __ESP8266_ZLIB_TCP_H__

#include "osapi.h"

#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "zlib.h"
#include "zlib_tcp.h"




//回调函数,tcp接收到数据后先调用此回调函数
//返回值: true: 继续执行json处理	false:不执行json处理
typedef bool (*zlib_tcp_received_callback_function)(void *arg, char *pusrdata, unsigned short length) ;


extern void zlib_tcp_init(uint16_t port);
extern void zlib_tcp_set_received_callback(zlib_tcp_received_callback_function cb);
extern void zlib_tcp_reply(void *arg, char *psend, uint16_t length);

#endif

