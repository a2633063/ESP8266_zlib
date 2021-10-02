#ifndef __ESP8266_ZLIB_FUNCTION_H__
#define __ESP8266_ZLIB_FUNCTION_H__

#include "user_interface.h"
#include "zlib.h"




extern void zlib_reboot_delay(int32_t time_out);
extern void zlib_fun_wifi_send(void *arg, Wifi_Comm_type_t type, char*topic, char *s, int qos, int retain);

#endif
