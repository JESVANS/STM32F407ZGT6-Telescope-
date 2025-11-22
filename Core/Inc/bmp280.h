#ifndef __BMP280_H__
#define __BMP280_H__

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define BMP280_I2C_ADDR    (0x76 << 1)   /* 默认 0x76，若为 0x77 请修改 */
#define BMP280_I2C_HANDLE  hi2c1         /* 使用 I2C1 */

#define BMP280_OK          0
#define BMP280_ERR_I2C     1
#define BMP280_ERR_CHIP    2
#define BMP280_ERR_PARAM   3

/* 初始化并读取校准数据，返回 BMP280_OK 成功 */
int BMP280_Init(void);

/* 读取温度（摄氏度）和气压（帕），两者可为 NULL */
int BMP280_ReadTempPressure(float *temp_c, float *press_pa);

/* 单独读取 */
int BMP280_ReadTemperature(float *temp_c);
int BMP280_ReadPressure(float *press_pa);

/* 根据气压计算海拔（sea_level_pa 可选，默认 101325 Pa） */
float BMP280_CalcAltitude(float pressure_pa, float sea_level_pa);

#endif /* __BMP280_H__ */
