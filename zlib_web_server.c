#include "stdlib.h"
#include "osapi.h"
#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "zlib.h"
#include "zlib_web_server.h"

URL_Http_Call_t *g_app_handlers;    //储存web相关信息
uint8_t g_app_handlers_count = 0;
static const char * content_type_string[] = { "text/html", "application/json" };    //与ContentType顺序同步
/**
 * 函  数  名: parse_url/save_data/save_data/check_data/data_send
 * 函数说明: http字符串处理解析
 * 参        数:
 * 返        回:
 */
/******************************************************************************
 * FunctionName : parse_url
 * Description  : parse the received data from the server
 * Parameters   : precv -- the received data
 *                purl_frame -- the result of parsing the url
 * Returns      : none
 *******************************************************************************/
static void ICACHE_FLASH_ATTR parse_url(char *precv, URL_Frame *purl_frame)
{
    char *str = NULL;
    uint8 length = 0;
    char *pbuffer = NULL;
    char *pbufer = NULL;

    if(purl_frame == NULL || precv == NULL)
    {
        return;
    }

    pbuffer = (char *) os_strstr(precv, "Host:");

    if(pbuffer != NULL)
    {
        length = pbuffer - precv;
        pbufer = (char *) os_zalloc(length + 1);
        pbuffer = pbufer;
        os_memcpy(pbuffer, precv, length);      //pbuffer 为第一行内容
        os_memset(purl_frame, 0, sizeof(struct URL_Frame));

        if(os_strncmp(pbuffer, "GET ", 4) == 0)
        {
            purl_frame->Type = GET;
            pbuffer += 4;
        }
        else if(os_strncmp(pbuffer, "POST ", 5) == 0)
        {
            purl_frame->Type = POST;
            pbuffer += 5;
        }

        //pbuffer++;
        str = (char *) os_strstr(pbuffer, "?");
        if(str != NULL)
        {   //获取Get下有参数的uri及参数
            length = str - pbuffer;
            os_memcpy(purl_frame->pUri, pbuffer, length);       //获取uri

            str = (char *) os_strstr(pbuffer, " HTTP");
            if(str != NULL)
            {
                pbuffer += length + 1;
                length = str - pbuffer;
                os_memcpy(purl_frame->pGetdat, pbuffer, length);        //获取uri中请求数据
            }
            else
            {
                purl_frame->Type = ERROR;
                return;
            }
        }
        else
        {   //获取Get下无参数的uri
            str = (char *) os_strstr(pbuffer, " HTTP");

            if(str != NULL)
            {
                length = str - pbuffer;
                os_memcpy(purl_frame->pUri, pbuffer, length);       //获取uri,uri中无请求数据
            }
            else
            {
                purl_frame->Type = ERROR;
                return;
            }
        }

        //获取post数据
        str = (char *) os_strstr(precv, "\r\n\r\n");
        if(str != NULL)
        {
            str += 4;
            length = os_strlen(precv) - (str - precv);
            os_memcpy(purl_frame->pPostdat, str, length);
        }

        os_free(pbufer);

        //获取Content-Type内容
        purl_frame->contentType = TEXT_HTML;    //默认为text/html
        str = (char *) os_strstr(precv, "Content-Type:");
        if(str != NULL)
        {
            str += 13;       //os_strlen("Content-Type:")
            if(*str == ' ') str++;
            if(os_strncmp(str, "application/json", 16) == 0)
            {
                purl_frame->contentType = APPLICATIOIN_JSON;
            }
            /*else if(os_strncmp(str, "POST ", 5) == 0)
             {
             }*/
        }
    }
    else
    {
        return;
    }
}

static char *precvbuffer;
static uint32 dat_sumlength = 0;
static bool ICACHE_FLASH_ATTR save_data(char *precv, uint16 length)
{
    bool flag = false;
    char length_buf[10] = { 0 };
    char *ptemp = NULL;
    char *pdata = NULL;
    uint16 headlength = 0;
    static uint32 totallength = 0;

    ptemp = (char *) os_strstr(precv, "\r\n\r\n");  //获取http 字符串head结尾

    if(ptemp != NULL)
    {
        length -= ptemp - precv;
        length -= 4;
        totallength += length;
        headlength = ptemp - precv + 4;
        pdata = (char *) os_strstr(precv, "Content-Length: ");

        if(pdata != NULL)
        {
            pdata += 16;
            precvbuffer = (char *) os_strstr(pdata, "\r\n");

            if(precvbuffer != NULL)
            {
                os_memcpy(length_buf, pdata, precvbuffer - pdata);
                dat_sumlength = atoi(length_buf);
            }
        }
        else
        {
            if(totallength != 0x00)
            {
                totallength = 0;
                dat_sumlength = 0;
                return false;
            }
        }
        if((dat_sumlength + headlength) >= 1024)
        {
            precvbuffer = (char *) os_zalloc(headlength + 1);
            os_memcpy(precvbuffer, precv, headlength + 1);
        }
        else
        {
            precvbuffer = (char *) os_zalloc(dat_sumlength + headlength + 1);
            os_memcpy(precvbuffer, precv, os_strlen(precv));
        }
    }
    else
    {
        if(precvbuffer != NULL)
        {
            totallength += length;
            os_memcpy(precvbuffer + os_strlen(precvbuffer), precv, length);
        }
        else
        {
            totallength = 0;
            dat_sumlength = 0;
            return false;
        }
    }

    if(totallength == dat_sumlength)
    {
        totallength = 0;
        dat_sumlength = 0;
        return true;
    }
    else
    {
        return false;
    }
}

static bool ICACHE_FLASH_ATTR check_data(char *precv, uint16 length)
{
    //bool flag = true;
    char length_buf[10] = { 0 };
    char *ptemp = NULL;
    char *pdata = NULL;
    char *tmp_precvbuffer;
    uint16 tmp_length = length;
    uint32 tmp_totallength = 0;

    ptemp = (char *) os_strstr(precv, "\r\n\r\n");

    if(ptemp != NULL)
    {
        tmp_length -= ptemp - precv;
        tmp_length -= 4;
        tmp_totallength += tmp_length;

        pdata = (char *) os_strstr(precv, "Content-Length: ");

        if(pdata != NULL)
        {
            pdata += 16;
            tmp_precvbuffer = (char *) os_strstr(pdata, "\r\n");

            if(tmp_precvbuffer != NULL)
            {
                os_memcpy(length_buf, pdata, tmp_precvbuffer - pdata);
                dat_sumlength = atoi(length_buf);
                LOGD("[ZLIB_WEB_SERVER]A_dat:%u,tot:%u,lenght:%u\n", dat_sumlength, tmp_totallength, tmp_length);
                if(dat_sumlength != tmp_totallength)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

/**
 * 函  数  名: _web_server_recv
 * 函数说明: web server tcp接收数据回调函数
 * 参        数:
 * 返        回: 无
 */
static void ICACHE_FLASH_ATTR _web_server_recv(void *arg, char *pusrdata, unsigned short length)
{
    URL_Frame *pURL_Frame = NULL;
    char *pParseBuffer = NULL;
    bool parse_flag = false;
    char i;
    struct espconn *ptrespconn = arg;

    LOGD("[ZLIB_WEB_SERVER]http tcp data len:%u\n", length);
    if(check_data(pusrdata, length) == false)
    {
//		LOGW("goto\n");
        goto _temp_exit;
    }

    if(precvbuffer != NULL)
    {
        os_free(precvbuffer);
        precvbuffer = NULL;
    }
    parse_flag = save_data(pusrdata, length);   //拼接多次tcp传输的一次http请求
    if(parse_flag == false)
    {
        zlib_web_server_reply(ptrespconn, false, TEXT_HTML, NULL);
    }

    pURL_Frame = (URL_Frame *) os_zalloc(sizeof(URL_Frame));
    parse_url(pusrdata, pURL_Frame);
    LOGD("[ZLIB_WEB_SERVER]Type: %d\n", pURL_Frame->Type);
    LOGD("[ZLIB_WEB_SERVER]Content-Type: %s\n", content_type_string[pURL_Frame->contentType]);
    LOGI("[ZLIB_WEB_SERVER]pUri: %s\n", pURL_Frame->pUri);
    LOGD("[ZLIB_WEB_SERVER]pGetdat: %s\n", pURL_Frame->pGetdat);
    LOGD("[ZLIB_WEB_SERVER]pPostdat: %s\n", pURL_Frame->pPostdat);

    //data_send(ptrespconn, true, pURL_Frame->contentType, pURL_Frame->pPostdat[0] == 0 ? NULL : pURL_Frame->pPostdat);
    for (i = 0; i < g_app_handlers_count; i++)
    {
        //os_strlen(pURL_Frame->pUri)
        if(os_strncmp(g_app_handlers[i].uri, pURL_Frame->pUri,
                ((os_strlen(pURL_Frame->pUri) > os_strlen(g_app_handlers[i].uri)) ?
                os_strlen(pURL_Frame->pUri) : os_strlen(g_app_handlers[i].uri))) == 0)
        {
            switch (pURL_Frame->Type)
            {
                case GET:
                    if(g_app_handlers[i].get_handler != NULL) g_app_handlers[i].get_handler(ptrespconn, pURL_Frame);
                    break;
                case POST:
                    if(g_app_handlers[i].set_handler != NULL) g_app_handlers[i].set_handler(ptrespconn, pURL_Frame);
                    break;

            }
            break;
        }
    }
    if(i >= sizeof(g_app_handlers) / sizeof(struct http_uri_call))
    {
        LOGW("[ZLIB_WEB_SERVER]no http web:%s\n", pURL_Frame->pUri);
        zlib_web_server_reply(ptrespconn, false, pURL_Frame->contentType, NULL);
    }

    os_free(pURL_Frame);
    pURL_Frame = NULL;
    _temp_exit: ;

}

/**
 * 函  数  名: zlib_web_server_init
 * 函数说明: web server 初始化
 * 参        数: uint16_t port:web server监听的端口
 * 返        回: 无
 */
void ICACHE_FLASH_ATTR zlib_web_server_init(uint16_t port, URL_Http_Call_t url_http_call[], uint8_t http_num)
{
    static struct espconn esp_conn;
    static esp_tcp esptcp;

    g_app_handlers = url_http_call;
    g_app_handlers_count = http_num;

    esp_conn.type = ESPCONN_TCP;
    esp_conn.state = ESPCONN_NONE;
    esp_conn.proto.tcp = &esptcp;
    esp_conn.proto.tcp->local_port = port;

    espconn_regist_recvcb(&esp_conn, _web_server_recv);

    espconn_accept(&esp_conn);
    LOGI("[ZLIB_WEB_SERVER]web_server_init, port:%d\n", port);

}

/**
 * 函  数  名: zlib_web_server_init
 * 函数说明: web server 回复http请求
 * 参        数:  arg -- argument to set for client or server
 *         responseOK -- true or false
 *         psend -- The send data
 * 返        回: 无
 */
void ICACHE_FLASH_ATTR zlib_web_server_reply(void *arg, bool responseOK, enum ContentType content_type, char *psend)
{
    uint16 length = 0;
    char *pbuf = NULL;
    char httphead[256];
    struct espconn *ptrespconn = arg;
    os_memset(httphead, 0, 256);

    if(responseOK)
    {
        os_sprintf(httphead, "HTTP/1.0 200 OK\r\nContent-Length: %d\r\n", psend ? os_strlen(psend) : 0);

        if(psend)
        {
            os_sprintf(httphead + os_strlen(httphead),
                    "Content-Type: %s;charset=UTF-8\r\nExpires: Fri, 10 Apr 2008 14:00:00 GMT\r\nPragma: no-cache\r\n\r\n",
                    content_type_string[content_type]);
            length = os_strlen(httphead) + os_strlen(psend);
            pbuf = (char *) os_zalloc(length + 1);
            os_memcpy(pbuf, httphead, os_strlen(httphead));
            os_memcpy(pbuf + os_strlen(httphead), psend, os_strlen(psend));
        }
        else
        {
            os_sprintf(httphead + os_strlen(httphead), "\n");
            length = os_strlen(httphead);
        }
    }
    else
    {
        os_sprintf(httphead, "HTTP/1.0 400 BadRequest\r\nContent-Length: 0\r\n\n");
        length = os_strlen(httphead);
    }

    if(psend)
    {
        espconn_sent(ptrespconn, pbuf, length);
    }
    else
    {
        espconn_sent(ptrespconn, httphead, length);
    }

    if(pbuf)
    {
        os_free(pbuf);
        pbuf = NULL;
    }
}
