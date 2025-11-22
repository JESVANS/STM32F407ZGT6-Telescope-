#ifndef __HC_SR04_H__
#define __HC_SR04_H__

#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef enum {
    HCSR04_OK = 0,
    HCSR04_ERR_TIMEOUT_RISING = 1,
    HCSR04_ERR_TIMEOUT_FALLING = 2,
    HCSR04_ERR_NOT_INIT = 3
} hcsr04_status_t;

/* 初始化：指定 TRIG 端口/引脚（输出）和 ECHO 端口/引脚（输入），
   echo_pull: GPIO_NOPULL / GPIO_PULLUP / GPIO_PULLDOWN */
void HC_SR04_Init(GPIO_TypeDef *trig_port, uint16_t trig_pin,
                  GPIO_TypeDef *echo_port, uint16_t echo_pin,
                  uint32_t echo_pull);

/* 发起一次测距，返回状态；若成功且 us_ptr 非 NULL，返回回波脉冲宽度（微秒）；
   若 cm_ptr 非 NULL，返回距离（厘米，浮点） */
hcsr04_status_t HC_SR04_Measure(uint32_t *us_ptr, float *cm_ptr);

/* 微调：设置测量超时（微秒），默认 30000us (30ms) */
void HC_SR04_SetTimeoutUs(uint32_t timeout_us);

#endif /* __HC_SR04_H__ */
