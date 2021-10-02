#include "zlib.h"
#include "zlib_function.h"
#include "zlib_ota.h"
#include "upgrade.h"

//延时重启,一定时间后重启设备
static os_timer_t _timer_reboot;
static void _reboot_timer_fun(void *arg)
{

    if(system_upgrade_flag_check() == UPGRADE_FLAG_FINISH)
    {
        LOGE("[ZLIB_FUNCTION]System Reboot for ota!\n\n");
        system_upgrade_reboot();
    }
    else
    {
        LOGE("[ZLIB_FUNCTION]System Reboot!\n\n");
        system_restart();
    }
}
void ICACHE_FLASH_ATTR zlib_reboot_delay(int32_t time_out)
{
    os_timer_disarm(&_timer_reboot);

    if(time_out < 0)
    {
        LOGI("[ZLIB_FUNCTION]System Reboot task cancel!\n\n");
        return;
    }
    else if(time_out == 0) _reboot_timer_fun(NULL);

    os_timer_setfn(&_timer_reboot, (os_timer_func_t *) _reboot_timer_fun, NULL);
    os_timer_arm(&_timer_reboot, time_out, false);
    LOGW("[ZLIB_FUNCTION]System Reboot after %d ms\n", time_out);
}

//根据通信内容,回复/发送数据
void ICACHE_FLASH_ATTR zlib_fun_wifi_send(void *arg, Wifi_Comm_type_t type, char*topic, char *s, int qos, int retain)
{
    if(s == NULL) return;

    switch (type)
    {
        case WIFI_COMM_TYPE_MQTT:
        {
            if(topic == NULL)
                zlib_mqtt_send_message("", s, qos, retain);
            else
                zlib_mqtt_send_message(topic, s, qos, retain);
            break;
        }
        case WIFI_COMM_TYPE_UDP:
        {
            zlib_udp_reply(arg, (char *) s);
            break;
        }
        case WIFI_COMM_TYPE_TCP:
        {
            zlib_tcp_reply(arg, (char *) s);
            break;
        }
        case WIFI_COMM_TYPE_HTTP:
        {
            zlib_web_server_reply(arg, true, APPLICATIOIN_JSON, (char *) s);
            break;
        }
    }
}
