#include "zlib.h"
#include "zlib_wifi.h"



static uint8_t hw_mac[6];
static uint8_t str_mac[16];
static uint8_t str_ip[16];
static state_wifi_state_t wifi_states = STATE_WIFI_STAMODE_IDE;
struct _zlib_wifi_callback_t
{
    uint32_t repeat_time;
    zlib_wifi_callback_function wifi_cb;
};
//wifi状态回调函数
static struct _zlib_wifi_callback_t _zlib_wifi_callback = { 0, NULL };

#if (ZLIB_WIFI_MDSN_ENABLI)
//mdns必须在ip获取后过一段时间再启动!
static os_timer_t _timer_mdns;
static void _user_wifi_mdns_timer_fun(void *arg)
{
    //启动mdns
    struct mdns_info info;
    char mdns_strName[32] = { 0 };
    char mdns_data_mac[32] = { 0 };
    char mdns_data_type[16] = { 0 };

    struct ip_info ipconfig;

    wifi_get_ip_info(STATION_IF, &ipconfig);

    os_memset(mdns_strName, 0, 32);
    os_memset(mdns_data_mac, 0, 32);
    os_memset(mdns_data_type, 0, 16);
    os_sprintf(mdns_strName, DEVICE_NAME, zlib_wifi_get_mac_str() + 8);
    info.host_name = mdns_strName;
    info.ipAddr = ipconfig.ip.addr; //ESP8266 station IP; //ESP8266 station IP
    info.server_name = "zcontrol";
    info.server_port = 10182;
    //info.txt_data[0] = "version = "VERSION;
    os_sprintf(mdns_data_mac, "mac = %s", zlib_wifi_get_mac_str());
    info.txt_data[0] = mdns_data_mac;
    os_sprintf(mdns_data_type, "type = %d", TYPE);
    info.txt_data[1] = mdns_data_type;
    espconn_mdns_init(&info);
}
#endif

#if (ZLIB_WIFI_CALLBACK_REPEAT) //wifi连接过程中 是否重复调用回调函数
static os_timer_t _timer_cb_repeat;
/**
 * 函 数 名: _zlib_wifi_cb_repeat
 * 函数说明: wifi连接过程  是否重复调用回调函数
 * 参    数: 无
 * 返    回: 无
 */
static void _zlib_wifi_cb_repeat(void *arg)
{
    if (wifi_get_opmode() != STATION_MODE || wifi_station_get_connect_status() == STATION_GOT_IP)
    {
        os_timer_disarm(&_timer_cb_repeat);
        return;
    }
    if (_zlib_wifi_callback.wifi_cb)
    {
        _zlib_wifi_callback.wifi_cb(NULL, true);
    }
}
/**
 * 函 数 名: _zlib_wifi_cb_repeat_start
 * 函数说明: 启动wifi连接过程  是否重复调用回调函数的定时器
 * 参    数: 无
 * 返    回: 无
 */
static void _zlib_wifi_cb_repeat_start(void)
{
    if (_zlib_wifi_callback.repeat_time > 0 && _zlib_wifi_callback.wifi_cb)
    {
        os_timer_disarm(&_timer_cb_repeat);
        os_timer_setfn(&_timer_cb_repeat, (os_timer_func_t *) _zlib_wifi_cb_repeat, NULL);
        os_timer_arm(&_timer_cb_repeat, _zlib_wifi_callback.repeat_time, true);
    }
}
#endif
/**
 * 函 数 名: _zlib_wifi_handle_event_cb
 * 函数说明: wifi状态回调函数
 * 参    数: System_Event_t *evt
 * 返    回: 无
 */
static void _zlib_wifi_handle_event_cb(System_Event_t *evt)
{
    switch (evt->event)
    {
    case EVENT_STAMODE_DISCONNECTED:
    {
        wifi_states = STATE_WIFI_STAMODE_DISCONNECTED;
#if defined(GPIO_WIFI_LED_IO_MUX) && defined(GPIO_WIFI_LED_IO_NUM)&& defined(GPIO_WIFI_LED_IO_FUNC)
        //若定义了wifi指示灯,则初始化灯
        wifi_status_led_install(GPIO_WIFI_LED_IO_NUM, GPIO_WIFI_LED_IO_MUX, GPIO_WIFI_LED_IO_FUNC);
#endif
#if (ZLIB_WIFI_CALLBACK_REPEAT)
        _zlib_wifi_cb_repeat_start();
#endif
        break;
    }
    case EVENT_STAMODE_GOT_IP:
    {
        wifi_states = STATE_WIFI_STAMODE_GOT_IP;
        LOGI("[ZLIB_WIFI]wifi got ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR, IP2STR(&evt->event_info.got_ip.ip),
                IP2STR(&evt->event_info.got_ip.mask), IP2STR(&evt->event_info.got_ip.gw));
        LOGI("\n");

        struct station_config wifi_config;
        wifi_station_get_config(&wifi_config);
        wifi_station_set_config(&wifi_config);
        os_sprintf(str_ip, IPSTR, IP2STR(&evt->event_info.got_ip.ip));

#if defined(GPIO_WIFI_LED_IO_MUX) && defined(GPIO_WIFI_LED_IO_NUM)&& defined(GPIO_WIFI_LED_IO_FUNC)
        wifi_status_led_uninstall();
#endif
#if (ZLIB_WIFI_MDSN_ENABLI)
        os_timer_disarm(&_timer_mdns);
        os_timer_setfn(&_timer_mdns, (os_timer_func_t *) _user_wifi_mdns_timer_fun, NULL);
        os_timer_arm(&_timer_mdns, 1000, false);
#endif
        break;
    }
    case EVENT_STAMODE_CONNECTED:
        wifi_states = STATE_WIFI_STAMODE_CONNECTED;
        break;
    case EVENT_SOFTAPMODE_STACONNECTED:
        wifi_states = STATE_WIFI_SOFTAPMODE;
        break;
    case EVENT_SOFTAPMODE_STADISCONNECTED:
        wifi_states = STATE_WIFI_SOFTAPMODE;
        break;
    default:
        break;
    }

    if (_zlib_wifi_callback.wifi_cb)
    {
        _zlib_wifi_callback.wifi_cb(evt, false);
    }
}
/**
 * 函 数 名: zlib_wifi_init
 * 函数说明: zlib库初始化wifi
 * 参    数: bool ap:初始化为ap模式,否则为station模式,(若无ssid保存则始终为ap模式)
 * 返    回: 无
 */
void ICACHE_FLASH_ATTR zlib_wifi_init(bool ap)
{
    uint8_t i;
    //设置为station模式
    wifi_set_opmode(STATION_MODE);

    //设置自动连接AP
    if (wifi_station_get_auto_connect() == 0)
    {
        wifi_station_set_auto_connect(1);
        LOGD("[ZLIB_WIFI]set default auto connect AP:true");
    }

    //设置wifi连接回调函数
    wifi_set_event_handler_cb(_zlib_wifi_handle_event_cb);

    //获取mac
    wifi_get_macaddr(STATION_IF, hw_mac);
    os_sprintf(str_mac, "%02x%02x%02x%02x%02x%02x", MAC2STR(hw_mac));
    LOGI("str_mac : %s \n", str_mac);

    //获取保存ssid数量
    struct station_config config[5];
    i = wifi_station_get_ap_info(config);
    LOGD("[ZLIB_WIFI]wifi info : %d \n", i);

    if (!ap && i > 0)
    {
        //wifi初始化完成,station模式,系统初始化完成后连接wifi
        LOGI("[ZLIB_WIFI]user_wifi_Station\n");
    }
    else
    {
        //wifi初始化完成,AP模式
        zlib_wifi_AP();
    }

#if defined(GPIO_WIFI_LED_IO_MUX) && defined(GPIO_WIFI_LED_IO_NUM)&& defined(GPIO_WIFI_LED_IO_FUNC)
    //若定义了wifi指示灯,则初始化灯
    wifi_status_led_install(GPIO_WIFI_LED_IO_NUM, GPIO_WIFI_LED_IO_MUX, GPIO_WIFI_LED_IO_FUNC);
#endif

}

/**
 * 函 数 名: zlib_wifi_set_callback
 * 函数说明  设置wifi状态改变时回调函数接口
 * 参   数: __zlib_wifi_cb回调函数接口
 * uint32_t repeat_time:在wifi连接过程中定时回调间隔,0为关闭.支持范围:见os_timer_arm范围
 * 返   回: 无
 */
void ICACHE_FLASH_ATTR zlib_wifi_set_callback(zlib_wifi_callback_function _zlib_wifi_cb_fun, uint32_t repeat_time)
{
    if (repeat_time < 5 || repeat_time > 0x68D7A3)
    {
        LOGE("[ZLIB_WIFI]zlib_wifi_set_callback: repeat_time range is [5,0x68D7A3]!");
        repeat_time = 0;
    }
    _zlib_wifi_callback.wifi_cb = _zlib_wifi_cb_fun;
    _zlib_wifi_callback.repeat_time = repeat_time;

#if (ZLIB_WIFI_CALLBACK_REPEAT)
    _zlib_wifi_cb_repeat_start();
#endif
}
/**
 * 函 数 名: zlib_wifi_AP
 * 函数说明  启动ap模式
 * 参   数: 无
 * 返   回: 无
 */
void ICACHE_FLASH_ATTR zlib_wifi_AP()
{
    char str[32] = { 0 };

    if (wifi_get_opmode() == STATIONAP_MODE) return;

    //设置为ap模式
    wifi_set_opmode_current(SOFTAP_MODE);

    //设置AP参数
    struct softap_config configAp;
    os_sprintf(configAp.ssid, DEVICE_NAME, zlib_wifi_get_mac_str() + 8); //设置ssid
    configAp.ssid_len = os_strlen(configAp.ssid);
    configAp.channel = 5;
    configAp.authmode = AUTH_OPEN;
    configAp.ssid_hidden = 0;
    configAp.max_connection = 4;
    configAp.beacon_interval = 100;
    wifi_softap_set_config(&configAp);

    //设置ip相关内容
    struct ip_info info;
    wifi_softap_dhcps_stop();
    IP4_ADDR(&info.ip, 192, 168, 0, 1);
    IP4_ADDR(&info.gw, 192, 168, 0, 1);
    IP4_ADDR(&info.netmask, 255, 255, 255, 0);
    wifi_set_ip_info(SOFTAP_IF, &info);
    wifi_softap_dhcps_start();
    wifi_states = STATE_WIFI_SOFTAPMODE;

    LOGI("[ZLIB_WIFI]user_wifi_AP, ssid:%s\n", configAp.ssid);
}

/**
 * 函 数 名: zlib_wifi_get_mac/zlib_wifi_get_mac_str
 * 函数说明  获取mac地址
 * 参   数: 无
 * 返   回: 无
 */
uint8_t * ICACHE_FLASH_ATTR zlib_wifi_get_mac(void)
{
    return hw_mac;
}
uint8_t * ICACHE_FLASH_ATTR zlib_wifi_get_mac_str(void)
{
    return str_mac;
}
state_wifi_state_t ICACHE_FLASH_ATTR zlib_wifi_get_state(void)
{
    return wifi_states;
}
