#ifndef __ESP8266_ZLIB_OTA_H__
#define __ESP8266_ZLIB_OTA_H__


typedef enum {
    STATE_OTA_RESULT_SUCCESS=0,
    STATE_OTA_RESULT_DNS_FAIL,
    STATE_OTA_RESULT_MEM_FAIL,
    STATE_OTA_RESULT_URL_FAIL,
    STATE_OTA_RESULT_FAIL,
    STATU_OTA_RESULT_MAX
} state_ota_result_state_t;


//回调函数,tcp接收到数据后先调用此回调函数
//返回值: true: 继续执行json处理 false:不执行json处理
typedef void (*zlib_ota_result_callback_function)(state_ota_result_state_t b) ;



extern int8_t zlib_ota_start(char *s);
extern int8_t zlib_ota_set_result_callback(zlib_ota_result_callback_function cb);
#endif
