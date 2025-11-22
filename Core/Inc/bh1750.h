#ifndef __BH1750_H__
#define __BH1750_H__

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define BH1750_ADDR        (0x23 << 1) /* 7-bit 0x23 -> HAL 8-bit */
#define BH1750_I2C_HANDLE  hi2c1

#define BH1750_OK          0
#define BH1750_ERR_I2C     1
#define BH1750_ERR_TIMEOUT 2

/* 初始化（上电后调用，包含开机和复位，可选） */
int BH1750_Init(void);

/* 开机/休眠/复位 */
int BH1750_PowerOn(void);
int BH1750_PowerDown(void);
int BH1750_Reset(void);

/* 设置测量模式（若需要）：常量参见下: 
    0x10 连续高分辨率模式（1 lx 分辨率，测量时间约 120ms）
    0x11 连续高分辨率模式2（0.5 lx 分辨率，测量时间约 120ms）
    0x13 连续低分辨率模式（4 lx 分辨率，测量时间约 16ms）
    0x20 单次高分辨率模式（1 lx 分辨率，测量时间约 120ms，测量后自动休眠）
    0x21 单次高分辨率模式2（0.5 lx 分辨率，测量时间约 120ms，测量后自动休眠）
    0x23 单次低分辨率模式（4 lx 分辨率，测量时间约 16ms，测量后自动休眠）
*/
int BH1750_SetMode(uint8_t mode);

/* 读取光照强度（单位 lux），使用单次高分辨率测量（one-time H-res mode）
   返回 BH1750_OK 成功并通过指针返回 lux，其他返回错误码 */
int BH1750_ReadLux(float *lux);

#endif /* __BH1750_H__ */
