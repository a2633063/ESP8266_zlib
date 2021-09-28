#ifndef __ESP8266_ZLIB_WEB_SERVER_H__
#define __ESP8266_ZLIB_WEB_SERVER_H__

#include "osapi.h"

#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "zlib.h"
#include "zlib_web_server.h"



#define URL_MAX_LENGTH 64
#define URL_GET_DAT_MAX_LENGTH 256
#define URL_POST_DAT_MAX_LENGTH 512
#define URLSize 10

typedef enum ProtocolType
{ //暂时仅实现GET POST 其他不支持
    ERROR = -1,
    GET = 0,
    POST,
    OPTIONS,
    HEAD,
    PUT,
    DELETE,
    TRACE,
    CONNECT
} ProtocolType;

typedef enum ContentType
{ //与数组content_type_string 顺序同步
    TEXT_HTML = 0,
    APPLICATIOIN_JSON
} ContentType;

typedef struct URL_Frame
{
    enum ProtocolType Type;
    enum ContentType contentType;
    char pUri[URL_MAX_LENGTH];
    char pGetdat[URL_GET_DAT_MAX_LENGTH];
    char pPostdat[URL_POST_DAT_MAX_LENGTH];

} URL_Frame;

//外部函数/定义
typedef struct URL_Http_Call_t
{
    /** URI of the WSGI */
    const char *uri;
    /** Indicator for HTTP headers to be sent in the response*/
//  int hdr_fields;
    /** Flag indicating if exact match of the URI is required or not */
    //int http_flags;
    /** HTTP GET or HEAD Handler */
    int (*get_handler)(void *arg, URL_Frame *purl_frame);
    /** HTTP POST Handler */
    int (*set_handler)(void *arg, URL_Frame *purl_frame);
    /** HTTP PUT Handler */
    int (*put_handler)(void *arg, URL_Frame *purl_frame);
    /** HTTP DELETE Handler */
    int (*delete_handler)(void *arg, URL_Frame *purl_frame);
} URL_Http_Call_t;



extern void zlib_web_server_init(uint16_t port);
extern void ICACHE_FLASH_ATTR zlib_web_server_reply(void *arg, bool responseOK, enum ContentType content_type, char *psend);

#endif

