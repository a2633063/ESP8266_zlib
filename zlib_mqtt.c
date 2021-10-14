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
#include "zlib_mqtt.h"

static os_timer_t _timer_mqtt;

static MQTT_Client mqttClient;
static zlib_mqtt_topic_info_t *_mqtt_topic = NULL;
static uint8_t _mqtt_topic_num = 0;
static zlib_mqtt_message_info_t *_mqtt_online_message = NULL;
static uint8_t _mqtt_online_message_num = 0;
static bool _mqtt_is_connected = false;

static zlib_mqtt_received_callback_function _mqtt_received = NULL;

/**
 * 函 数 名: _mqtt_timer_func
 * 函数说明: mqtt定时器回调函数.用户在wifi连接成功后连接mqtt.
 *        在mqtt连接成功后订阅主题,在mqtt连接成功后发送设备上线信息.在心跳包超时时间后发送更新上线信息
 * 参    数:
 * 返    回: 无
 */
static void ICACHE_FLASH_ATTR _mqtt_timer_func(void *arg)
{
    static uint16_t status = 0;
    MQTT_Client* client = (MQTT_Client*) arg;
    uint8_t i;
    switch (status)
    {
        case 0:
        {   //等待wifi连接
            if(wifi_get_opmode() != STATION_MODE) return;
            if(zlib_wifi_get_state() != STATE_WIFI_STAMODE_GOT_IP) return;
            zlib_mqtt_connect();
            status++;
            break;
        }
        case 1:
        {
            if(zlib_mqtt_is_connected() == false) return;
            LOGI("[ZLIB_MQTT]mqtt connected!\n");
            status++;
            break;
        }
        case 2:
        case 3:
        {
            if(_mqtt_online_message != NULL)
            {
                for (i = 0; i < _mqtt_online_message_num; i++)
                {
                    bool b = zlib_mqtt_send_message(_mqtt_online_message[i].topic, _mqtt_online_message[i].message,
                            _mqtt_online_message[i].qos, _mqtt_online_message[i].retain);

                    LOGI("[ZLIB_MQTT]send online message to mqtt[%s]:%s\n", _mqtt_online_message[i].topic,
                            _mqtt_online_message[i].message);
                }
                //if(!b) status = 0;
                status++;
                if(status == 3)
                {
                    os_timer_disarm(&_timer_mqtt);
                    os_timer_setfn(&_timer_mqtt, (os_timer_func_t *) _mqtt_timer_func, arg);
                    os_timer_arm(&_timer_mqtt, client->connect_info.keepalive * 2000, true);
                }
                else
                {
                    os_timer_disarm(&_timer_mqtt);
                    os_timer_setfn(&_timer_mqtt, (os_timer_func_t *) _mqtt_timer_func, arg);
                    os_timer_arm(&_timer_mqtt, 1000, true);
                }
            }
            else
            {
                status = 4;
                //os_timer_disarm(&_timer_mqtt);
                //os_timer_setfn(&_timer_mqtt, (os_timer_func_t *) _mqtt_timer_func, arg);
                //os_timer_arm(&_timer_mqtt, 1000, true);
            }
            break;
        }
        case 4:
        default:
            status = 0;
            os_timer_disarm(&_timer_mqtt);
            break;
    }
}
/**
 * 函 数 名: _mqtt_con_received
 * 函数说明: mqtt接收数据回调函数
 * 参    数: 
 * 返    回: 无
 */
static void ICACHE_FLASH_ATTR _mqtt_con_received(uint32_t *arg, const char* topic, uint32_t topic_len, const char *data,
        uint32_t data_len)
{
    MQTT_Client* client = (MQTT_Client*) arg;

    char *topicBuf = (char*) os_zalloc(topic_len + 1);
    char *dataBuf = (char*) os_zalloc(data_len + 1);

    if(topicBuf == NULL || dataBuf == NULL)
    {
        LOGE("[ZLIB_MQTT]memory error!\n");
        LOGE("[ZLIB_MQTT]MQTT message lost:[%s][ZLIB_MQTT]     \n[%s]\n", topic, data);
        return;
    }
    os_memcpy(topicBuf, topic, topic_len);
    topicBuf[topic_len] = 0;

    os_memcpy(dataBuf, data, data_len);
    dataBuf[data_len] = 0;

    if(_mqtt_received != NULL)
    {
        bool b = _mqtt_received(arg, topicBuf, topic_len, dataBuf, data_len);
        if(!b) goto EXIT;
    }
    zlib_json_deal(arg, WIFI_COMM_TYPE_MQTT, dataBuf, (void *) topicBuf);

    EXIT:
    os_free(topicBuf);
    os_free(dataBuf);
    return;
}
static void _mqtt_connected_cb(uint32_t *args)
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

    _mqtt_is_connected = true;

}
static void _mqtt_disconnected_cb(uint32_t *args)
{
    _mqtt_is_connected = false;
    //MQTT_Client* client = (MQTT_Client*) args;
    LOGE("[ZLIB_MQTT]mqtt disconnected\r\n");
    os_timer_disarm(&_timer_mqtt);
    os_timer_setfn(&_timer_mqtt, (os_timer_func_t *) _mqtt_timer_func, args);
    os_timer_arm(&_timer_mqtt, 1000, true);
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

    if((host == NULL || mqtt_info == NULL || port == 0) || (mqtt_info->client_id == NULL ) || (os_strlen(host) < 2))
    {
        LOGW("[ZLIB_MQTT]mqtt info error, mqtt deinit\n");
        return;
    }

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

    os_timer_disarm(&_timer_mqtt);
    os_timer_setfn(&_timer_mqtt, (os_timer_func_t *) _mqtt_timer_func, &mqttClient);
    os_timer_arm(&_timer_mqtt, 1000, true);
    LOGI("[ZLIB_MQTT]mqtt init %s:%d\n", host, port);
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
    _mqtt_topic_num = count;
}
/**
 * 函  数  名: zlib_mqtt_set_online_message
 * 函数说明: 设置上线主题数据,在设备连上mqtt后立即发送,延时心跳时间后再发送一次
 * 参        数: p:上线主题数据
 * 返        回: 无
 */
void ICACHE_FLASH_ATTR zlib_mqtt_set_online_message(zlib_mqtt_message_info_t *p, uint8_t count)
{
    _mqtt_online_message_num = count;
    _mqtt_online_message = p;
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
/**
 * 函  数  名: zlib_mqtt_connect/zlib_mqtt_disconnect
 * 函数说明: 连接mqtt/断开mqtt
 * 参        数: 无
 * 返        回: 无
 */
void ICACHE_FLASH_ATTR zlib_mqtt_connect(void)
{
    MQTT_Connect(&mqttClient);
}
void ICACHE_FLASH_ATTR zlib_mqtt_disconnect(void)
{
    MQTT_Disconnect(&mqttClient);
}

/**
 * 函  数  名: zlib_mqtt_send_message
 * 函数说明: mqtt发送数据
 * 参        数: topic,message,qos,retain
 * 返        回: 无
 */
bool ICACHE_FLASH_ATTR zlib_mqtt_send_message(char *topic, char* message, int qos, int retain)
{
    if(qos < 0)
        qos = 0;
    else if(qos > 2) qos = 2;
    return zlib_mqtt_is_connected ? MQTT_Publish(&mqttClient, topic, message, os_strlen(message), qos, retain) : false;
}

/**
 * 函  数  名: zlib_mqtt_send_byte
 * 函数说明: mqtt发送数据
 * 参        数: topic,message,qos,retain
 * 返        回: 无
 */
bool ICACHE_FLASH_ATTR zlib_mqtt_send_byte(char *topic, char* message, int data_length, int qos, int retain)
{
    if(qos < 0)
        qos = 0;
    else if(qos > 2) qos = 2;
    return zlib_mqtt_is_connected ? MQTT_Publish(&mqttClient, topic, message, data_length, qos, retain) : false;
}
