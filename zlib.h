#ifndef __ESP8266_ZLIB_H__
#define __ESP8266_ZLIB_H__

#include "osapi.h"

#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "zlib_wifi.h"

#ifndef DEVICE_NAME	//���Ȳ��ɳ���31
	#define DEVICE_NAME "zControl_%s"
#endif

#if (1)	//����������
//LOGD	0
//LOGI	1
//LOGW	2
//LOGE	3
#define DEBUG_LEVEL	0	//0��ʾȫ��log 4������κ�log


#if (DEBUG_LEVEL < 1)
	#define LOGD( format, ... )    os_printf( format, ## __VA_ARGS__ )
#else
	#define LOGD( format, ... )
#endif

#if (DEBUG_LEVEL < 2)
	#define LOGI( format, ... )    os_printf( format, ## __VA_ARGS__ )
#else
	#define LOGI( format, ... )
#endif

#if (DEBUG_LEVEL < 3)
	#define LOGW( format, ... )    os_printf( format, ## __VA_ARGS__ )
#else
	#define LOGW( format, ... )
#endif

#if (DEBUG_LEVEL < 4)
	#define LOGE( format, ... )    os_printf( format, ## __VA_ARGS__ )
#else
	#define LOGE( format, ... )
#endif

#endif



#endif
