#include "bmp280.h"
#include "main.h"
#include <string.h>
#include <math.h>

extern I2C_HandleTypeDef BMP280_I2C_HANDLE; /* hi2c2 by header macro */

typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
} bmp280_calib_t;

static bmp280_calib_t calib;
static int32_t t_fine = 0;

/* Registers */
#define REG_ID          0xD0
#define REG_RESET       0xE0
#define REG_CTRL_MEAS   0xF4
#define REG_CONFIG      0xF5
#define REG_PRESS_MSB   0xF7
#define REG_TEMP_MSB    0xFA
#define REG_CALIB_START 0x88

/* 高精度配置：
   osrs_t = 5 (x16)
   osrs_p = 5 (x16)
   mode   = 3 (normal)
   CTRL_MEAS = (osrs_t<<5) | (osrs_p<<2) | mode = 0xB7

   CONFIG:
   t_sb (standby) = 100 (500 ms) -> bits[7:5] = 100 (4)
   filter = 100 (x16) -> bits[4:2] = 100 (4)
   spi3w_en = 0
   CONFIG = (4<<5) | (4<<2) = 0x90
*/
#define BMP280_CTRL_MEAS_HIGH_PREC  0xB7
#define BMP280_CONFIG_HIGH_PREC     0x90

static HAL_StatusTypeDef write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return HAL_I2C_Master_Transmit(&BMP280_I2C_HANDLE, BMP280_I2C_ADDR, buf, 2, 200);
}

static HAL_StatusTypeDef read_regs(uint8_t reg, uint8_t *buf, uint16_t len)
{
    if (HAL_I2C_Master_Transmit(&BMP280_I2C_HANDLE, BMP280_I2C_ADDR, &reg, 1, 200) != HAL_OK) return HAL_ERROR;
    return HAL_I2C_Master_Receive(&BMP280_I2C_HANDLE, BMP280_I2C_ADDR, buf, len, 300);
}

static void parse_calib(uint8_t *buf)
{
    calib.dig_T1 = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
    calib.dig_T2 = (int16_t)buf[2] | ((int16_t)buf[3] << 8);
    calib.dig_T3 = (int16_t)buf[4] | ((int16_t)buf[5] << 8);
    calib.dig_P1 = (uint16_t)buf[6] | ((uint16_t)buf[7] << 8);
    calib.dig_P2 = (int16_t)buf[8] | ((int16_t)buf[9] << 8);
    calib.dig_P3 = (int16_t)buf[10] | ((int16_t)buf[11] << 8);
    calib.dig_P4 = (int16_t)buf[12] | ((int16_t)buf[13] << 8);
    calib.dig_P5 = (int16_t)buf[14] | ((int16_t)buf[15] << 8);
    calib.dig_P6 = (int16_t)buf[16] | ((int16_t)buf[17] << 8);
    calib.dig_P7 = (int16_t)buf[18] | ((int16_t)buf[19] << 8);
    calib.dig_P8 = (int16_t)buf[20] | ((int16_t)buf[21] << 8);
    calib.dig_P9 = (int16_t)buf[22] | ((int16_t)buf[23] << 8);
}

/* 按 Bosch 数据手册补偿温度，返回摄氏度 *100 (centi-deg)，并设置 t_fine */
static int32_t compensate_temp(int32_t adc_T)
{
    int64_t var1, var2;
    var1 = ((((int64_t)adc_T >> 3) - ((int64_t)calib.dig_T1 << 1)) * (int64_t)calib.dig_T2) >> 11;
    var2 = (((((int64_t)adc_T >> 4) - (int64_t)calib.dig_T1) * (((int64_t)adc_T >> 4) - (int64_t)calib.dig_T1)) >> 12) * (int64_t)calib.dig_T3 >> 14;
    t_fine = (int32_t)(var1 + var2);
    int32_t T = (t_fine * 5 + 128) >> 8; /* T in 0.01 degC */
    return T;
}

/* 补偿气压，输入 adc_P，返回 Pa (浮点) */
static float compensate_press(int32_t adc_P)
{
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib.dig_P3) >> 8) + ((var1 * (int64_t)calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1) * (int64_t)calib.dig_P1) >> 33;

    if (var1 == 0) return 0.0f; /* avoid division by zero */

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = ((int64_t)calib.dig_P9 * (p >> 13) * (p >> 13)) >> 25;
    var2 = ((int64_t)calib.dig_P8 * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib.dig_P7) << 4);

    /* p is in Q24.8 format; convert to Pa */
    return (float)p / 256.0f;
}

int BMP280_Init(void)
{
    uint8_t id;
    uint8_t buf[24];

    /* read chip id */
    if (read_regs(REG_ID, &id, 1) != HAL_OK) return BMP280_ERR_I2C;
    if (id != 0x58) return BMP280_ERR_CHIP; /* BMP280 chip id 0x58 */

    /* read calibration 0x88..0xA1 (24 bytes) */
    if (read_regs(REG_CALIB_START, buf, 24) != HAL_OK) return BMP280_ERR_I2C;
    parse_calib(buf);

    /* configure: 高精度模式 */
    if (write_reg(REG_CTRL_MEAS, BMP280_CTRL_MEAS_HIGH_PREC) != HAL_OK) return BMP280_ERR_I2C;
    if (write_reg(REG_CONFIG, BMP280_CONFIG_HIGH_PREC) != HAL_OK) return BMP280_ERR_I2C;

    HAL_Delay(10);
    return BMP280_OK;
}

int BMP280_ReadTempPressure(float *temp_c, float *press_pa)
{
    uint8_t buf[6];
    if (read_regs(REG_PRESS_MSB, buf, 6) != HAL_OK) return BMP280_ERR_I2C;

    int32_t adc_P = (int32_t)(((uint32_t)buf[0] << 12) | ((uint32_t)buf[1] << 4) | ((uint32_t)buf[2] >> 4));
    int32_t adc_T = (int32_t)(((uint32_t)buf[3] << 12) | ((uint32_t)buf[4] << 4) | ((uint32_t)buf[5] >> 4));

    /* 温度补偿 -> 返回 0.01 degC */
    int32_t T100 = compensate_temp(adc_T);
    if (temp_c) *temp_c = T100 / 100.0f;

    if (press_pa) *press_pa = compensate_press(adc_P);

    return BMP280_OK;
}

int BMP280_ReadTemperature(float *temp_c)
{
    return BMP280_ReadTempPressure(temp_c, NULL);
}

int BMP280_ReadPressure(float *press_pa)
{
    return BMP280_ReadTempPressure(NULL, press_pa);
}

float BMP280_CalcAltitude(float pressure_pa, float sea_level_pa)
{
    if (pressure_pa <= 0.0f) return 0.0f;
    if (sea_level_pa <= 0.0f) sea_level_pa = 101325.0f;
    return 44330.0f * (1.0f - powf(pressure_pa / sea_level_pa, 0.19029495718363465f));
}