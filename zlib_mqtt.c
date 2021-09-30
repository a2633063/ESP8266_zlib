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
#include "mqtt.h"

static MQTT_Client mqttClient;
static zlib_mqtt_topic_info_t *_mqtt_topic = NULL;
static uint8_t _mqtt_topic_num=0;
static bool _mqtt_is_connected = false;

static zlib_mqtt_received_callback_function _mqtt_received = NULL;
/**
 * 函 数 名: _mqtt_con_received
 * 函数说明: mqtt接收数据回调函数
 * 参    数: 
 * 返    回: 无
 */
void ICACHE_FLASH_ATTR _mqtt_con_received(uint32_t *arg, const char* topic, uint32_t topic_len, const char *data,
        uint32_t data_len)
{
    MQTT_Client* client = (MQTT_Client*) arg;

    char *topicBuf = (char*) os_zalloc(topic_len + 1);
    char *dataBuf = (char*) os_zalloc(data_len + 1);

    os_memcpy(topicBuf, topic, topic_len);
    topicBuf[topic_len] = 0;

    os_memcpy(dataBuf, data, data_len);
    dataBuf[data_len] = 0;

    if(_mqtt_received != NULL)
    {
        bool b = _mqtt_received(arg, topicBuf, dataBuf);
        if(!b) return;
    }
    zlib_json_deal(arg, WIFI_COMM_TYPE_MQTT, dataBuf, (void *) topicBuf);
}
void _mqtt_connected_cb(uint32_t *args)
{
    uint8_t i;
    MQTT_Client* client = (MQTT_Client*) args;
    LOGI("MQTT: Connected\r\n");
    if(_mqtt_topic != NULL)
    {
        LOGD("[ZLIB_MQTT]Subscribe topic[%d]:\r\n", _mqtt_topic_num);
        for (i = 0; i < _mqtt_topic_num; i++)
        {
            MQTT_Subscribe(client, _mqtt_topic[i].topic, _mqtt_topic[i].qos);
            LOGD("[ZLIB_MQTT] %d: %s,[%d]:\r\n", i, _mqtt_topic[i].topic, _mqtt_topic[i].qos);
        }
    }

//    os_timer_disarm(&timer_mqtt);
//    os_timer_setfn(&timer_mqtt, (os_timer_func_t *) user_mqtt_timer_func, NULL);
//    os_timer_arm(&timer_mqtt, MQTT_TIMER_REPEATTIME, 1);
    _mqtt_is_connected = true;

}
void _mqtt_disconnected_cb(uint32_t *args)
{
    _mqtt_is_connected = false;
//    os_timer_disarm(&timer_mqtt);
    MQTT_Client* client = (MQTT_Client*) args;
    LOGE("[ZLIB_MQTT]mqtt disconnected\r\n");
}
/**
 * 函  数  名: zlib_mqtt_init
 * 函数说明: zlib库初始化mqtt
 * 参        数: host:mqtt服务器地址
 *         port:mqtt服务器端口
 *         mqtt_info:mqtt连接相关信息
 * 返        回: 无
 */
void ICACHE_FLASH_ATTR zlib_mqtt_init(char *host, uint16_t port, mqtt_connect_info_t *mqtt_info)
{
    MQTT_InitConnection(&mqttClient, host, port, NO_TLS);

    MQTT_InitClient(&mqttClient, mqtt_info->client_id, mqtt_info->username, mqtt_info->password, mqtt_info->keepalive,
            mqtt_info->clean_session);

    if(mqtt_info->will_topic != NULL && mqtt_info->will_message != NULL)
    {
        MQTT_InitLWT(&mqttClient, mqtt_info->will_topic, mqtt_info->will_message, mqtt_info->will_qos,
                mqtt_info->will_retain);
    }
    MQTT_OnConnected(&mqttClient, _mqtt_connected_cb);
    MQTT_OnDisconnected(&mqttClient, _mqtt_disconnected_cb);
//    MQTT_OnPublished(&mqttClient, mqttPublishedCb);
    MQTT_OnData(&mqttClient, _mqtt_con_received);

    LOGI("[ZLIB_MQTT]mqtt init port:%d\n", port);
}

/**
 * 函 数 名: zlib_mqtt_set_received_callback
 * 函数说明: 设置mqtt接收回调函数
 * 参    数:
 * 返    回: true: 继续执行json处理	false:不执行json处理
 */
void ICACHE_FLASH_ATTR zlib_mqtt_set_received_callback(zlib_mqtt_received_callback_function cb)
{
    _mqtt_received = cb;
}
/**
 * 函  数  名: zlib_mqtt_subscribe
 * 函数说明: 订阅主题,此函数仅保存需要订阅的主题,在mqtt连接后自动订阅
 * 参        数: p:需要订阅的主题字符串数组
 * 返        回: 无
 */
void ICACHE_FLASH_ATTR zlib_mqtt_subscribe(zlib_mqtt_topic_info_t *p, uint8_t count)
{
    _mqtt_topic = p;
    _mqtt_topic_num= count;
}
/**
 * 函  数  名: zlib_mqtt_reply
 * 函数说明: 回复mqtt请求
 * 参        数:  arg -- argument to set for client or server
 *         psend -- The send data
 * 返        回: 无
 */
void ICACHE_FLASH_ATTR zlib_mqtt_reply(void *arg, char *psend)
{

}
/**
 * 函  数  名: zlib_mqtt_is_connected
 * 函数说明: mqtt是否有连接
 * 参        数:  arg -- argument to set for client or server
 *         psend -- The send data
 * 返        回: 无
 */
bool ICACHE_FLASH_ATTR zlib_mqtt_is_connected(void)
{
    return _mqtt_is_connected;
}
