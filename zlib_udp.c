#include "stdlib.h"
#include "osapi.h"
#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "zlib.h"
#include "zlib_udp.h"
#include "zlib_function.h"

static struct espconn _ptrespconn;
static zlib_udp_received_callback_function _udp_received = NULL;
/**
 * 函 数 名: _udp_con_received
 * 函数说明: udp接收数据回调函数
 * 参    数: port:udp监听的端口
 * 返    回: 无
 */
void ICACHE_FLASH_ATTR _udp_con_received(void *arg, char *pusrdata, unsigned short length)
{
    struct espconn *pesp_conn = arg;
//    LOGD("[ZLIB_UDP]udp received:ip:%d.%d.%d.%d:%d\n",pesp_conn->proto.udp->remote_ip[0]
//               ,pesp_conn->proto.udp->remote_ip[1]
//               ,pesp_conn->proto.udp->remote_ip[2]
//               ,pesp_conn->proto.udp->remote_ip[3]
//               ,pesp_conn->proto.udp->remote_port);
    if(_udp_received != NULL)
    {
        bool b = _udp_received(arg, pusrdata, length);
        if(!b) return;
    }
    zlib_json_deal(arg, WIFI_COMM_TYPE_UDP, pusrdata, NULL);
}
/**
 * 函 数 名: zlib_udp_init
 * 函数说明: zlib库初始化udp
 * 参    数: port:udp监听的端口
 * 返    回: 无
 */
void ICACHE_FLASH_ATTR zlib_udp_init(uint16_t port)
{
    _ptrespconn.type = ESPCONN_UDP;
    if(_ptrespconn.proto.udp == NULL) _ptrespconn.proto.udp = (esp_udp *) os_zalloc(sizeof(esp_udp));
    _ptrespconn.proto.udp->local_port = port;
    espconn_regist_recvcb(&_ptrespconn, _udp_con_received);
    espconn_create(&_ptrespconn);

    LOGI("[ZLIB_UDP]udp init port:%d\n", port);
}

/**
 * 函 数 名: zlib_udp_set_received_callback
 * 函数说明: 设置udp接收回调函数
 * 参    数: 
 * 返    回: true: 继续执行json处理	false:不执行json处理
 */
void ICACHE_FLASH_ATTR zlib_udp_set_received_callback(zlib_udp_received_callback_function cb)
{

    _udp_received = cb;
}

/**
 * 函  数  名: zlib_udp_reply
 * 函数说明: 回复tcp请求
 * 参        数:  arg -- argument to set for client or server
 *         psend -- The send data
 * 返        回: 无
 */
void ICACHE_FLASH_ATTR zlib_udp_reply(void *arg, char *psend)
{
    struct espconn *pesp_conn;
    if(arg == NULL)
    {
        pesp_conn = &_ptrespconn;
    }
    else
    {
        pesp_conn = arg;
    }
//    LOGD("[ZLIB_UDP]udp reply:ip:%d.%d.%d.%d:%d\n",pesp_conn->proto.udp->remote_ip[0]
//            ,pesp_conn->proto.udp->remote_ip[1]
//            ,pesp_conn->proto.udp->remote_ip[2]
//            ,pesp_conn->proto.udp->remote_ip[3]
//            ,pesp_conn->proto.udp->remote_port);
    pesp_conn->proto.udp->remote_port = ZLIB_UDP_REPLY_PORT;  //端口
    pesp_conn->proto.udp->remote_ip[0] = 255;   //udp广播ַ
    pesp_conn->proto.udp->remote_ip[1] = 255;
    pesp_conn->proto.udp->remote_ip[2] = 255;
    pesp_conn->proto.udp->remote_ip[3] = 255;
    espconn_send(pesp_conn, psend, os_strlen(psend));
}

