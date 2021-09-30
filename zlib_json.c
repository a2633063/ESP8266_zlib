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
 * 函  数  名: zlib_json_deal
 * 函数说明: 处理json数据,供tcp/udp/mqtt/http调用
 * 参        数: arg:通信相关指针
 *          type:通信类型
 *          jsonStr:需要处理的json字符串
 *          p:通信附带的其他数据   mqtt:topic字符串     http:get数据(可能为空字符串)
 * 返        回: 无
 */
void ICACHE_FLASH_ATTR zlib_json_deal(void *arg, Wifi_Comm_type_t type, char* jsonStr, void *p)
{
    if(jsonStr == NULL) return;
    if(os_strlen(jsonStr) < 2) return;

    cJSON *pJsonRoot = cJSON_Parse(jsonStr);
    if(pJsonRoot == NULL) return;

    //todo 增加"cmd":"device report"处理
    if(_json_deal != NULL) _json_deal(arg, type, pJsonRoot,p);
    cJSON_Delete(pJsonRoot);
}
