#include "zlib.h"
#include "zlib_function.h"

static os_timer_t _timer_reboot;
static void _reboot_timer_fun(void *arg)
{
    LOGE("[ZLIB_FUNCTION]System Reboot!\n\n");
    system_restart();
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

    LOGW("[ZLIB_FUNCTION]System Reboot after %d ms\n",time_out);

}
