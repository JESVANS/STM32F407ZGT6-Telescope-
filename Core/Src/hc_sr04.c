#include "hc_sr04.h"
#include "main.h" /* for SystemCoreClock and HAL */

#include <stdbool.h>

static GPIO_TypeDef *hc_trig_port = NULL;
static uint16_t hc_trig_pin = 0;
static GPIO_TypeDef *hc_echo_port = NULL;
static uint16_t hc_echo_pin = 0;
static uint32_t hc_timeout_us = 30000; /* default 30 ms */

static uint32_t dwt_scale = 0; /* SystemCoreClock / 1_000_000 */

static inline void dwt_init_once(void)
{
    if (dwt_scale != 0) return;
    /* enable DWT */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    dwt_scale = SystemCoreClock / 1000000U;
    if (dwt_scale == 0) dwt_scale = 1;
}

static inline uint32_t dwt_get_us(void)
{
    return DWT->CYCCNT / dwt_scale;
}

static void dwt_delay_us(uint32_t us)
{
    uint32_t start = dwt_get_us();
    while ((dwt_get_us() - start) < us) { __NOP(); }
}

void HC_SR04_SetTimeoutUs(uint32_t timeout_us)
{
    hc_timeout_us = timeout_us;
}

void HC_SR04_Init(GPIO_TypeDef *trig_port, uint16_t trig_pin,
                  GPIO_TypeDef *echo_port, uint16_t echo_pin,
                  uint32_t echo_pull)
{
    /* save pins */
    hc_trig_port = trig_port;
    hc_trig_pin = trig_pin;
    hc_echo_port = echo_port;
    hc_echo_pin = echo_pin;

    /* init DWT */
    dwt_init_once();

    /* configure TRIG as output push-pull */
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = hc_trig_pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(hc_trig_port, &gpio);
    HAL_GPIO_WritePin(hc_trig_port, hc_trig_pin, GPIO_PIN_RESET);

    /* configure ECHO as input with specified pull */
    gpio.Pin = hc_echo_pin;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = (echo_pull == GPIO_PULLUP) ? GPIO_PULLUP :
                (echo_pull == GPIO_PULLDOWN) ? GPIO_PULLDOWN : GPIO_NOPULL;
    HAL_GPIO_Init(hc_echo_port, &gpio);
}

hcsr04_status_t HC_SR04_Measure(uint32_t *us_ptr, float *cm_ptr)
{
    if (hc_trig_port == NULL || hc_echo_port == NULL) return HCSR04_ERR_NOT_INIT;

    /* ensure trigger low */
    HAL_GPIO_WritePin(hc_trig_port, hc_trig_pin, GPIO_PIN_RESET);
    dwt_delay_us(2);

    /* send 10us pulse */
    HAL_GPIO_WritePin(hc_trig_port, hc_trig_pin, GPIO_PIN_SET);
    dwt_delay_us(10);
    HAL_GPIO_WritePin(hc_trig_port, hc_trig_pin, GPIO_PIN_RESET);

    uint32_t start_tick = dwt_get_us();
    uint32_t timeout = hc_timeout_us;

    /* wait for rising edge */
    while (HAL_GPIO_ReadPin(hc_echo_port, hc_echo_pin) == GPIO_PIN_RESET)
    {
        if ((dwt_get_us() - start_tick) > timeout) return HCSR04_ERR_TIMEOUT_RISING;
    }
    uint32_t t1 = dwt_get_us();

    /* wait for falling edge */
    while (HAL_GPIO_ReadPin(hc_echo_port, hc_echo_pin) == GPIO_PIN_SET)
    {
        if ((dwt_get_us() - t1) > timeout) return HCSR04_ERR_TIMEOUT_FALLING;
    }
    uint32_t t2 = dwt_get_us();

    uint32_t pulse_us = (t2 - t1);

    if (us_ptr) *us_ptr = pulse_us;
    if (cm_ptr)
    {
        /* distance cm = (pulse_us * speed_of_sound_cm_per_us) / 2
           speed_of_sound ~ 34300 cm/s = 0.0343 cm/us */
        *cm_ptr = (pulse_us * 0.0343f) / 2.0f;
    }

    return HCSR04_OK;
}