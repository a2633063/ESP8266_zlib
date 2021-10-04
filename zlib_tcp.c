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

static struct espconn _ptrespconn;
static zlib_tcp_received_callback_function _tcp_received = NULL;
/**
 * 函 数 名: _tcp_con_received
 * 函数说明: tcp接收数据回调函数
 * 参    数: port:tcp监听的端口
 * 返    回: 无
 */
void ICACHE_FLASH_ATTR _tcp_con_received(void *arg, char *pusrdata, unsigned short length)
{
    struct espconn *pesp_conn = arg;
    if(_tcp_received != NULL)
    {
        bool b = _tcp_received(arg, pusrdata, length);
        if(!b) return;
    }
    zlib_json_deal(arg, WIFI_COMM_TYPE_TCP, pusrdata, NULL);
}
/**
 * 函 数 名: zlib_tcp_init
 * 函数说明: zlib库初始化tcp
 * 参    数: port:tcp监听的端口
 * 返    回: 无
 */
void ICACHE_FLASH_ATTR zlib_tcp_init(uint16_t port)
{
    static esp_tcp esptcp;

    _ptrespconn.type = ESPCONN_TCP;
    _ptrespconn.state = ESPCONN_NONE;
    _ptrespconn.proto.tcp = &esptcp;
    _ptrespconn.proto.tcp->local_port = port;
    espconn_regist_recvcb(&_ptrespconn, _tcp_con_received);
    espconn_accept(&_ptrespconn);
    LOGI("[ZLIB_TCP]tcp init port:%d\n", port);
}

/**
 * 函 数 名: zlib_tcp_set_received_callback
 * 函数说明: 设置tcp接收回调函数
 * 参    数: 
 * 返    回: true: 继续执行json处理	false:不执行json处理
 */
void ICACHE_FLASH_ATTR zlib_tcp_set_received_callback(zlib_tcp_received_callback_function cb)
{
    _tcp_received = cb;
}
/**
 * 函  数  名: zlib_tcp_reply
 * 函数说明: 回复tcp请求
 * 参        数:  arg -- argument to set for client or server
 *         psend -- The send data
 * 返        回: 无
 */
void ICACHE_FLASH_ATTR zlib_tcp_reply(void *arg, char *psend)
{
    if(arg == NULL)
    {
        arg = &_ptrespconn;
    }
    espconn_send(arg, psend, os_strlen(psend));
}
