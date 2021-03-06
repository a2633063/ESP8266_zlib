#ifndef __ESP8266_ZLIB_WIFI_H__
#define __ESP8266_ZLIB_WIFI_H__

#include "user_interface.h"

#ifndef ZLIB_WIFI_MDSN_ENABLI
#define ZLIB_WIFI_MDSN_ENABLI  (0)  //是否启用mdns,默认不启用mdns
#endif

#ifndef ZLIB_WIFI_CALLBACK_REPEAT
#define ZLIB_WIFI_CALLBACK_REPEAT  (0)  //wifi连接过程中 是否重复调用回调函数
#endif

//WIFI状态指示灯定义
//#define ZLIB_WIFI_STATE_LED_IO_MUX     PERIPHS_IO_MUX_MTDI_U
//#define ZLIB_WIFI_STATE_LED_IO_NUM     12
//#define ZLIB_WIFI_STATE_LED_IO_FUNC    FUNC_GPIO12

#ifndef ZLIB_WIFI_CONNECTED_STATE_LED
#define ZLIB_WIFI_CONNECTED_STATE_LED  (0)  //wifi连接后 wifi状态指示灯io口电平
#endif

//回调函数
//is_repeat: true由定时器反复调用回调	falsewifi状态回调
typedef void (*zlib_wifi_callback_function)(System_Event_t *evt, bool is_repeat);

typedef enum {
    STATE_WIFI_STAMODE_IDE = 0,
    STATE_WIFI_STAMODE_DISCONNECTED,
    STATE_WIFI_STAMODE_CONNECTING,
    STATE_WIFI_STAMODE_CONNECTED,
    STATE_WIFI_STAMODE_GOT_IP,
    STATE_WIFI_SOFTAPMODE,
    STATE_WIFI_SOFTAPMODE_CONNECTED,
    STATE_WIFI_SMARTCONFIG,
    STATU_WIFI_MAX
} state_wifi_state_t;


extern void zlib_wifi_init(bool ap);
extern void zlib_wifi_AP(void);
extern void zlib_wifi_set_ssid(char *ssid, char * password);
extern void zlib_wifi_set_ssid_delay(char *ssid, char * password, uint16_t time_out);
extern uint8_t * zlib_wifi_get_mac(void);
extern uint8_t * zlib_wifi_get_mac_str(void);
extern uint8_t * ICACHE_FLASH_ATTR zlib_wifi_get_ip_str(void);
extern state_wifi_state_t zlib_wifi_get_state(void);
extern void ICACHE_FLASH_ATTR zlib_wifi_mac_init(void);
#endif
