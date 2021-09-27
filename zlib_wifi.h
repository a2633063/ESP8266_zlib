#ifndef __ESP8266_ZLIB_WIFI_H__
#define __ESP8266_ZLIB_WIFI_H__

#include "user_interface.h"

#ifndef ZLIB_WIFI_MDSN_ENABLI
#define ZLIB_WIFI_MDSN_ENABLI  (0)  //�Ƿ�����mdns,Ĭ�ϲ�����mdns
#endif

#ifndef ZLIB_WIFI_CALLBACK_REPEAT
#define ZLIB_WIFI_CALLBACK_REPEAT  (0)  //wifi���ӹ����� �Ƿ��ظ����ûص�����
#endif

//WIFI״ָ̬ʾ�ƶ���
//#define GPIO_WIFI_LED_IO_MUX     PERIPHS_IO_MUX_MTDI_U
//#define GPIO_WIFI_LED_IO_NUM     12
//#define GPIO_WIFI_LED_IO_FUNC    FUNC_GPIO12


//�ص�����
//is_repeat: true�ɶ�ʱ���������ûص�	falsewifi״̬�ص�
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
extern void ICACHE_FLASH_ATTR zlib_wifi_AP(void);
extern uint8_t * ICACHE_FLASH_ATTR zlib_wifi_get_mac(void);
extern uint8_t * ICACHE_FLASH_ATTR zlib_wifi_get_mac_str(void);
extern state_wifi_state_t ICACHE_FLASH_ATTR zlib_wifi_get_state(void);
#endif
