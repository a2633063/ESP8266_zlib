#ifndef __ESP8266_ZLIB_SETTING_H__
#define __ESP8266_ZLIB_SETTING_H__

#include "user_config.h"

#ifndef ZLIB_SETTING_SAVE_ADDR
#define ZLIB_SETTING_SAVE_ADDR  (0x70)  //储存地址
#endif

extern SpiFlashOpResult zlib_setting_save_config(void *arg, uint32_t length);
extern SpiFlashOpResult zlib_setting_get_config(void *arg, uint32_t length);
extern SpiFlashOpResult zlib_setting_save_flash(uint16_t addr, void *arg, uint32_t length);
extern SpiFlashOpResult zlib_setting_get_flash(uint16_t addr, void *arg, uint32_t length);
#endif
