#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"

#include "zlib.h"
#include "user_config.h"
#include "zlib_setting.h"

/**
 * 函  数  名: zlib_setting_save_config
 * 函数说明: 保存数据到flash
 * 参        数: arg需要保存的数据 类型
 *        length 长度
 * 返        回: 无
 */
SpiFlashOpResult ICACHE_FLASH_ATTR zlib_setting_save_config(void *arg, uint32_t length)
{
    SpiFlashOpResult r;
    uint32_t l = length;
    if(l % 4 != 0) l += 4 - l % 4;	// 4 字节对齐。
    uint8_t *p = (uint8_t *) os_malloc(length);
    os_memcpy(p, arg, l);
    r = spi_flash_erase_sector(ZLIB_SETTING_SAVE_ADDR);
    if(r != SPI_FLASH_RESULT_OK) goto EXIT;
    r = spi_flash_write(ZLIB_SETTING_SAVE_ADDR * 4096, (uint32 *) p, length);

    EXIT:
    os_free(p);
    if(r != SPI_FLASH_RESULT_OK) LOGE("[ZLIB_SETTING]save config fail! err:%d",r);
    return r;
}

/**
 * 函  数  名: zlib_setting_get_config
 * 函数说明: 从flash中获取数据
 * 参        数: arg获取到的数据
 *        length 长度
 * 返        回: 无
 */
SpiFlashOpResult ICACHE_FLASH_ATTR zlib_setting_get_config(void *arg, uint32_t length)
{

    SpiFlashOpResult r;

    uint32_t l = length;
    if(l % 4 != 0) l += 4 - l % 4;	// 4 字节对齐。
    uint8_t *p = (uint8_t *) os_malloc(l);

    r = spi_flash_read(ZLIB_SETTING_SAVE_ADDR * 4096, (uint32 *) p, l);

    if(r != SPI_FLASH_RESULT_OK) goto EXIT;
    os_memcpy(arg, p, length);

    EXIT:
    os_free(p);
    if(r != SPI_FLASH_RESULT_OK) LOGE("[ZLIB_SETTING]get config fail! err:%d",r);
    return r;
}

