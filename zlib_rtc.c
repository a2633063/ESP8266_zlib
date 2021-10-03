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

static os_timer_t _timer_rtc;
zlib_rtc_recall_callback_function _rtc_recall_cb = NULL;
/**
 * 函 数 名: _rtc_timer_func
 * 函数说明: rtc定时器回调函数.在秒钟变化后,调用回调函数
 * 参    数:
 * 返    回: 无
 */
static void ICACHE_FLASH_ATTR _rtc_timer_func(void *arg)
{
    static bool init_flag = false;   //判断是否有连接wifi成功过
    static uint64_t current_stamp_last = 0;
    if(!init_flag && zlib_wifi_get_state() != STATE_WIFI_STAMODE_GOT_IP)
    {
        return;
    }

    uint64_t current_stamp_temp = sntp_get_current_timestamp();
    if(current_stamp_temp > 0 && current_stamp_last != current_stamp_temp)
    {
        init_flag = true;
        current_stamp_last = current_stamp_temp;
        if(_rtc_recall_cb != NULL)
        {
            struct_time_t time;
            zlib_rtc_get_time(current_stamp_temp, &time);
            _rtc_recall_cb((const struct_time_t) time);

        }
    }
}
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

    os_memset(time, 0, sizeof(struct_time_t));
    if(current_stamp_temp == 0)
    {
//        time->year = 0;
//        time->month = 0;
//        time->day = 0;
//        time->week = 0;
//        time->hour = 0;
//        time->minute = 0;
//        time->second = 0;
        return;
    }
    //LOGI("[ZLIB_RTC]RTC time:%d\n", current_stamp_temp);
    //current_stamp_temp = +zlib_rtc_get_timezone * 3600;//默认已经考虑时区问题
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
    sntp_setservername(1, "ntp.sjtu.edu.cn"); // set server 1 by domain name
    sntp_setservername(2, "time1.aliyun.com"); // set server 2 by domain name
    //sntp_get_real_time();
    sntp_init();
    LOGI("[ZLIB_RTC]RTC init\n");
}

void ICACHE_FLASH_ATTR zlib_rtc_set_recall_callback(zlib_rtc_recall_callback_function cb)
{
    _rtc_recall_cb = cb;
    os_timer_disarm(&_timer_rtc);
    if(_rtc_recall_cb != NULL)
    {
        os_timer_setfn(&_timer_rtc, (os_timer_func_t *) _rtc_timer_func, NULL);
        os_timer_arm(&_timer_rtc, 400, true);
    }
}
/**
 * 函 数 名: zlib_rtc_set_timezone
 * 函数说明: 设置时区
 * 参    数: timezone 时区,范围[-11,13]
 * 返    回: 无
 */
void ICACHE_FLASH_ATTR zlib_rtc_set_timezone(int8_t timezone)
{
    if(sntp_set_timezone(timezone))
    {
        LOGD("set time zone %d\n", timezone);
    }
    else
        LOGD("set time zone %d error\n", timezone);
}

/**
 * 函 数 名: zlib_rtc_get_timezone
 * 函数说明: 获取当前时区
 * 参    数: 无
 * 返    回: 当前时区
 */
int8_t ICACHE_FLASH_ATTR zlib_rtc_get_timezone(void)
{
    return sntp_get_timezone();
}
