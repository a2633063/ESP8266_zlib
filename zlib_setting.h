#ifndef __ESP8266_ZLIB_SETTING_H__
#define __ESP8266_ZLIB_SETTING_H__

#ifndef ZLIB_SETTING_SAVE_ADDR
#define ZLIB_SETTING_SAVE_ADDR  (0x70)  //储存地址
#endif

extern SpiFlashOpResult ICACHE_FLASH_ATTR zlib_setting_save_config(void *arg, uint32_t length);
extern SpiFlashOpResult ICACHE_FLASH_ATTR zlib_setting_get_config(void *arg, uint32_t length);

#endif
