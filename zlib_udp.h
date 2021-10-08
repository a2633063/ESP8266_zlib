#ifndef __ESP8266_ZLIB_UDP_H__
#define __ESP8266_ZLIB_UDP_H__

#include "osapi.h"

#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "zlib.h"
#include "zlib_udp.h"

#ifndef ZLIB_UDP_REPLY_PORT
#define ZLIB_UDP_REPLY_PORT  (10181)  //udp回复默认端口号
#endif

//回调函数,udp接收到数据后先调用此回调函数
//返回值: true: 继续执行json处理	false:不执行json处理
typedef bool (*zlib_udp_received_callback_function)(void *arg, char *pusrdata, unsigned short length);

extern void zlib_udp_init(uint16_t port);
extern void zlib_udp_set_received_callback(zlib_udp_received_callback_function cb);
extern void zlib_udp_reply(void *arg, char *psend, uint16_t length);

#endif

