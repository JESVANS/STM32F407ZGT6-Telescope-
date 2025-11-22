#include "bh1750.h"
#include "main.h"
#include <string.h>

extern I2C_HandleTypeDef BH1750_I2C_HANDLE; /* hi2c1 by default */

#define BH1750_CMD_POWER_DOWN       0x00
#define BH1750_CMD_POWER_ON         0x01
#define BH1750_CMD_RESET            0x07
/* Measurement modes */
#define BH1750_ONE_TIME_H_RES       0x20 /* 1 lx resolution, measurement time ~120-180ms */
#define BH1750_CONT_H_RES           0x10

static HAL_StatusTypeDef bh_write(uint8_t cmd)
{
    return HAL_I2C_Master_Transmit(&BH1750_I2C_HANDLE, BH1750_ADDR, &cmd, 1, 200);
}

int BH1750_Init(void)
{
    if (BH1750_PowerOn() != BH1750_OK) return BH1750_ERR_I2C;
    HAL_Delay(10);
    if (BH1750_Reset() != BH1750_OK) return BH1750_ERR_I2C;
    HAL_Delay(10);
    return BH1750_OK;
}

int BH1750_PowerOn(void)
{
    if (bh_write(BH1750_CMD_POWER_ON) == HAL_OK) return BH1750_OK;
    return BH1750_ERR_I2C;
}

int BH1750_PowerDown(void)
{
    if (bh_write(BH1750_CMD_POWER_DOWN) == HAL_OK) return BH1750_OK;
    return BH1750_ERR_I2C;
}

int BH1750_Reset(void)
{
    /* Reset only valid when power on */
    if (bh_write(BH1750_CMD_RESET) == HAL_OK) return BH1750_OK;
    return BH1750_ERR_I2C;
}

int BH1750_SetMode(uint8_t mode)
{
    if (bh_write(mode) == HAL_OK) return BH1750_OK;
    return BH1750_ERR_I2C;
}

int BH1750_ReadLux(float *lux)
{
    uint8_t rx[2];
    HAL_StatusTypeDef ret;

    /* Send one-time high resolution measurement */
    ret = bh_write(BH1750_ONE_TIME_H_RES);
    if (ret != HAL_OK) return BH1750_ERR_I2C;

    /* 等待测量完成：datasheet 建议 max 180ms（使用120ms以上较好） */
    HAL_Delay(180);

    /* 读取 2 字节数据 */
    ret = HAL_I2C_Master_Receive(&BH1750_I2C_HANDLE, BH1750_ADDR, rx, 2, 300);
    if (ret != HAL_OK) return BH1750_ERR_I2C;

    uint16_t raw = ((uint16_t)rx[0] << 8) | rx[1];

    /* 转换为 lux：value / 1.2 (datasheet) */
    if (lux) *lux = (float)raw / 1.2f;

    return BH1750_OK;
}