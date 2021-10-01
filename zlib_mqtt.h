#ifndef __ESP8266_ZLIB_MQTT_H__
#define __ESP8266_ZLIB_MQTT_H__

#include "osapi.h"

#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "mqtt_msg.h"
#include "zlib.h"
#include "zlib_mqtt.h"

typedef struct zlib_mqtt_topic_info
{
    char* topic;
    int qos;
} zlib_mqtt_topic_info_t;

typedef struct zlib_mqtt_message_info
{
    char* topic;
    char* message;
    int qos;
    int retain;
} zlib_mqtt_message_info_t;

//回调函数,tcp接收到数据后先调用此回调函数
//返回值: true: 继续执行json处理	false:不执行json处理
typedef bool (*zlib_mqtt_received_callback_function)(uint32_t *arg, const char* topic, const char *data);

extern void zlib_mqtt_init(char *host, uint16_t port, mqtt_connect_info_t *mqtt_info);
extern void zlib_mqtt_set_received_callback(zlib_mqtt_received_callback_function cb);
//extern void zlib_mqtt_reply(void *arg, char *psend);
extern void zlib_mqtt_subscribe(zlib_mqtt_topic_info_t *p, uint8_t count);
extern void zlib_mqtt_set_online_message(zlib_mqtt_message_info_t *p, uint8_t count);
extern bool zlib_mqtt_is_connected(void);
extern void zlib_mqtt_connect(void);
extern void zlib_mqtt_disconnect(void);
extern bool zlib_mqtt_send_message(char *topic, char* message, int qos, int retain);







#endif

