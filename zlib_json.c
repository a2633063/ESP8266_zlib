#include "stdlib.h"
#include "osapi.h"
#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "../cJson/cJSON.h"
#include "zlib.h"
#include "zlib_json.h"
#include "zlib_tcp.h"
#include "zlib_udp.h"
//#include "zlib_mqtt.h"
#include "zlib_web_server.h"
#include "zlib_function.h"

static struct espconn _ptrespconn;
static zlib_json_deal_callback_function _json_deal = NULL;

/**
 * 函 数 名: zlib_json_init
 * 函数说明: zlib库初始化json回调函数
 * 参    数: port:tcp监听的端口
 * 返    回: 无
 */
void ICACHE_FLASH_ATTR zlib_json_init(zlib_json_deal_callback_function cb)
{
    _json_deal=cb;
}

/**
 * 函 数 名: zlib_json_deal
 * 函数说明: 处理json数据,供tcp/udp/mqtt/http调用
 * 参    数: type:调用此函数的通信接口库类型	jsonRoot:需要处理的json字符串
 * 返    回: 无
 */
void ICACHE_FLASH_ATTR zlib_json_deal(void *arg, Wifi_Comm_type_t type, char* jsonRoot)
{
    if(jsonRoot == NULL) return;
    if(os_strlen(jsonRoot) < 2) return;

    cJSON *pJsonRoot = cJSON_Parse(jsonRoot);
    if(pJsonRoot == NULL) return;

    if(_json_deal != NULL) _json_deal(arg, type, pJsonRoot);
    cJSON_Delete(pJsonRoot);
}
