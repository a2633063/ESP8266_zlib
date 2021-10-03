#ifndef __ESP8266_ZLIB_RTC_H__
#define __ESP8266_ZLIB_RTC_H__

#include "osapi.h"

#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "zlib.h"

enum
{
    Monday = 1,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday,
};
enum
{
    January = 1,
    February,
    March,
    April,
    May,
    June,
    July,
    August,
    September,
    October,
    November,
    December,
};
typedef struct struct_time
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t week;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

} struct_time_t;

//回调函数,tcp接收到数据后先调用此回调函数
//返回值: true: 继续执行json处理	false:不执行json处理
typedef bool (*zlib_tcp_received_callback_function)(void *arg, char *pusrdata, unsigned short length);

extern void zlib_rtc_init();
extern void zlib_rtc_set_received_callback(zlib_tcp_received_callback_function cb);
extern void zlib_rtc_reply(void *arg, char *psend);

extern void zlib_rtc_get_time(uint64 current_stamp_temp, struct_time_t *time);
#endif

