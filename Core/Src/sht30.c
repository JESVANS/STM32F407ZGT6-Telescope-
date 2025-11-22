#include "sht30.h"
#include "main.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

extern I2C_HandleTypeDef hi2c1; /* 使用 I2C1 */

double Temperature = 0.0;
double Humidity = 0.0;

/* CRC8 (polynomial 0x31, init 0xFF) */
static uint8_t sht30_crc8(const uint8_t *data, int len)
{
    uint8_t crc = 0xFF;
    const uint8_t POLY = 0x31;
    for (int i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (int b = 0; b < 8; b++)
        {
            crc = (crc & 0x80) ? (crc << 1) ^ POLY : (crc << 1);
        }
    }
    return crc;
}

/* 发送 16 位命令到 SHT30，返回 0 成功，非0 错误 */
char SHT31_Write_mode(uint16_t dat)
{
    uint8_t cmd[2];
    cmd[0] = (uint8_t)(dat >> 8);
    cmd[1] = (uint8_t)(dat & 0xFF);

    if (HAL_I2C_Master_Transmit(&hi2c1, SHT30_I2C_ADDR, cmd, 2, 100) != HAL_OK)
    {
        return 1;
    }
    return 0;
}

/* 读取命令 dat（读取6字节：T.MSB T.LSB T.CRC H.MSB H.LSB H.CRC）
   返回 0 成功并更新全局 Temperature/Humidity；非0 为错误码 */
char SHT30_Read(uint16_t dat)
{
    uint8_t cmd[2];
    uint8_t buf[6];

    cmd[0] = (uint8_t)(dat >> 8);
    cmd[1] = (uint8_t)(dat & 0xFF);

    /* 发送测量命令 */
    if (HAL_I2C_Master_Transmit(&hi2c1, SHT30_I2C_ADDR, cmd, 2, 100) != HAL_OK)
    {
        return 1;
    }

    /* 测量完成所需时间因命令而异；高重复性单次测量一般需要 ~15ms，保守使用20ms */
    HAL_Delay(20);

    /* 读取6字节数据 */
    if (HAL_I2C_Master_Receive(&hi2c1, SHT30_I2C_ADDR, buf, 6, 200) != HAL_OK)
    {
        return 2;
    }

    /* CRC 校验 */
    if (sht30_crc8(buf, 2) != buf[2]) return 3;
    if (sht30_crc8(buf + 3, 2) != buf[5]) return 4;

    uint16_t rawT = ((uint16_t)buf[0] << 8) | buf[1];
    uint16_t rawH = ((uint16_t)buf[3] << 8) | buf[4];

    /* 按数据手册转换 */
    double t = -45.0 + 175.0 * ((double)rawT / 65535.0);
    double h = 100.0 * ((double)rawH / 65535.0);

    Temperature = t;
    Humidity = h;

    return 0;
}

/* 友好单次测量接口：使用命令 0x2C06（高重复性，无时钟拉伸） */
char SHT30_Read_SingleShot(float *temp, float *hum)
{
    char ret = SHT30_Read(0x2C06);
    if (ret == 0)
    {
        if (temp) *temp = (float)Temperature;
        if (hum)  *hum  = (float)Humidity;
    }
    return ret;
}

/* 简单检测：发送软复位并检查 I2C 总线应答，返回 true 表示通信正常 */
bool SHT30_Check(void)
{
    // 先检测是否在线
    if (HAL_I2C_IsDeviceReady(&hi2c1, SHT30_I2C_ADDR, 2, 100) != HAL_OK)
        return false;

    uint8_t cmd[2] = {0x30, 0xA2}; /* soft reset */
    if (HAL_I2C_Master_Transmit(&hi2c1, SHT30_I2C_ADDR, cmd, 2, 100) == HAL_OK)
    {
        HAL_Delay(10);
        return true;
    }
    return false;
}
