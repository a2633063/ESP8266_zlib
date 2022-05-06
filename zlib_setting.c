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
    
    return zlib_setting_save_flash(ZLIB_SETTING_SAVE_ADDR,arg,length);
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
    return zlib_setting_get_flash(ZLIB_SETTING_SAVE_ADDR,arg,length);
}

/**
 * 函  数  名: zlib_setting_save_flash
 * 函数说明: 保存数据到flash
 * 参        数: arg需要保存的数据 类型
 *        length 长度
 * 返        回: 无
 */
SpiFlashOpResult ICACHE_FLASH_ATTR zlib_setting_save_flash(uint16_t addr, void *arg, uint32_t length)
{
    SpiFlashOpResult r;
    uint8_t i;
    uint8_t *temp_p = (uint8_t *) os_malloc(length);
    if(zlib_setting_get_flash(addr,temp_p, length) == SPI_FLASH_RESULT_OK)
    {
        if(os_memcmp(arg, temp_p, length) == 0)
        {
            LOGI("[ZLIB_SETTING]Same flash.don't need to save\n");
            os_free(temp_p);
            return SPI_FLASH_RESULT_OK;
        }
    }
    os_free(temp_p);

    uint32_t l = length;
    if(l % 4 != 0) l += 4 - l % 4;  // 4 字节对齐。
    uint8_t *p = (uint8_t *) os_malloc(l);
    if(p == NULL)
    {
        LOGE("[ZLIB_SETTING]get config fail\n");
        return -3;
    }
    os_memset(p, 0xff, l);
    os_memcpy(p, arg, length);

    uint8_t sector_num = l / 4096 + 1;    //需要写入/擦除几个扇区
    LOGI("[ZLIB_SETTING]save to Flash size:%d,  sector num:%d\n", length, sector_num);
    for (i = 0; i < sector_num; i++)
    {
        r = spi_flash_erase_sector(addr + i);
        if(r != SPI_FLASH_RESULT_OK) goto EXIT;
    }

    r = spi_flash_write(addr * 4096, (uint32 *) p, l);

    EXIT:
    os_free(p);
    if(r != SPI_FLASH_RESULT_OK)
        LOGE("[ZLIB_SETTING]save flash fail! err:%d\n", r);
    else
        LOGI("[ZLIB_SETTING]save flash success\n");
    return r;
}

/**
 * 函  数  名: zlib_setting_get_addr
 * 函数说明: 从flash中获取数据
 * 参        数: arg获取到的数据
 *        length 长度
 * 返        回: 无
 */
SpiFlashOpResult ICACHE_FLASH_ATTR zlib_setting_get_flash(uint16_t addr, void *arg, uint32_t length)
{

    SpiFlashOpResult r;

    uint32_t l = length;
    if(l % 4 != 0) l += 4 - l % 4;  // 4 字节对齐。

    uint8_t *p = (uint8_t *) os_malloc(l);
    if(p == NULL)
    {
        LOGE("[ZLIB_SETTING]get config fail\n");
        return -3;
    }
    r = spi_flash_read(addr * 4096, (uint32 *) p, l);

    if(r != SPI_FLASH_RESULT_OK) goto EXIT;

    os_memset(arg, 0xff, length);
    os_memcpy(arg, p, length);

    EXIT:
    os_free(p);
    if(r != SPI_FLASH_RESULT_OK)
        LOGE("[ZLIB_SETTING]get flash fail! err:%d\n", r);
    else
        LOGI("[ZLIB_SETTING]get flash success\n");
    return r;
}
