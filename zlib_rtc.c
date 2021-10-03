#include "stdlib.h"
#include "osapi.h"
#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "zlib.h"
#include "zlib_tcp.h"
#include "zlib_function.h"
#include "zlib_rtc.h"

#include "sntp.h"

#define isleap(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)

/**
 * 函 数 名: zlib_rtc_get_time
 * 函数说明: 获取当前rtc实时时间
 * 参    数:
 * 返    回: 无
 */
void ICACHE_FLASH_ATTR zlib_rtc_get_time(uint64 current_stamp_temp, struct_time_t *time)
{
    const uint8_t month_day[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
//    uint64_t current_stamp_temp = sntp_get_current_timestamp();
    if(current_stamp_temp == 0)
    {
        time->year = 0;
        time->month = 0;
        time->day = 0;
        time->week = 0;
        time->hour = 0;
        time->minute = 0;
        time->second = 0;
        return;

    }
    //LOGI("[ZLIB_RTC]RTC time:%d\n", current_stamp_temp);

    //时间戳转为年月日时分秒
    time->second = current_stamp_temp % 60; //获取秒
    current_stamp_temp /= 60;
    time->minute = current_stamp_temp % 60; //获取分
    current_stamp_temp /= 60;
    time->hour = current_stamp_temp % 24; //获取时
    current_stamp_temp /= 24;
    uint32_t day = current_stamp_temp; //获取多少天

    time->week = (day % 7 + 4) % 7; //获取周  //1970年1月1日为周四
    if(time->week == 0) time->week = 7;

    LOGI("[ZLIB_RTC]week:%d\n",time->week);
    uint16_t Pass4year = day / 1461;   //几个4年      4年总共1461天
    day %= 1461; //多余的年份对应天数
    time->year = 1970 + Pass4year * 4;

    while (1)
    {
        uint16_t day_pre_year = 365;
        if(isleap(time->year))
        {
            day_pre_year = 366;
        }
        if(day < day_pre_year) break;
        time->year++;
        day -= day_pre_year;
    }

    uint8_t leap_day = 0;
    if(isleap(time->year)) leap_day = 1;
    time->month = 0;
    while (1)
    {
        if(day < (month_day[time->month] + ((time->month == 1) ? leap_day : 0))) break;
        day = day - month_day[time->month] - ((time->month == 1) ? leap_day : 0);
        time->month++;
    }

    time->month++;
    time->day = day + 1;
}

/**
 * 函 数 名: zlib_rtc_init
 * 函数说明: rtc初始化(sntp初始化)
 * 参    数: port:tcp监听的端口
 * 返    回: 无
 */
void ICACHE_FLASH_ATTR zlib_rtc_init()
{
   sntp_setservername(0, "ntp1.aliyun.com"); // set server 0 by domain name
    //sntp_setservername(0, "zip-zhang.synology.me"); // set server 0 by domain name
     //sntp_setservername(0, "time1.aliyun.com"); // set server 0 by domain name
    sntp_setservername(2, "us.pool.ntp.org"); // set server 1 by domain name
    sntp_setservername(1, "ntp.sjtu.edu.cn"); // set server 2 by domain name
    //sntp_set_timezone();
    //sntp_get_real_time();
    sntp_init();
    LOGI("[ZLIB_RTC]RTC init\n");

}
