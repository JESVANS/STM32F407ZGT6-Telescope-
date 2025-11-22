#ifndef _BSP_SHT30_H_
#define _BSP_SHT30_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

extern double Temperature;
extern double Humidity;

#define SHT30_I2C_ADDR   (0x44 << 1)   // 7bit地址0x44 -> HAL使用8位地址
#define SHT30_I2C_HANDLE hi2c1         // 使用 I2C1

/* 保留原接口：在周期模式下写入测量模式命令 */
char SHT31_Write_mode(uint16_t dat);

/* 兼容接口：发送命令并读取返回（6字节），返回0表示成功 */
char SHT30_Read(uint16_t dat);

/* 更友好的单次测量接口（高重复性），通过指针返回 float 值 */
char SHT30_Read_SingleShot(float *temp, float *hum);

/* 检测函数：简单发送命令判断 I2C 总线应答，返回 true 表示设备可通信 */
bool SHT30_Check(void);

#endif /* _BSP_SHT30_H_ */
