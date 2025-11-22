/**
  ******************************************************************************
  * @file    bluetooth.c
  * @author  Your Name
  * @brief   JDY-31 Bluetooth Module Driver Implementation
  ******************************************************************************
  */

#include "bluetooth.h"
#include "usart.h"
#include <string.h>

/* 默认实例 */
BT_Handle_t hBluetooth = {
    .huart       = BT_DEFAULT_UART,
    .EN_Port     = BT_DEFAULT_EN_PORT,
    .EN_Pin      = BT_DEFAULT_EN_PIN,
    .STATE_Port  = BT_DEFAULT_STATE_PORT,
    .STATE_Pin   = BT_DEFAULT_STATE_PIN,
    .baudrate    = BT_DEFAULT_BAUDRATE,
    .initialized = 0
};

/* 内部帮助：若传入 NULL 使用默认实例 */
static inline BT_Handle_t* _handle(BT_Handle_t *hbt) {
    return hbt ? hbt : &hBluetooth;
}

BT_Status_t BT_Init(BT_Handle_t *hbt_in)
{
    BT_Handle_t *hbt = _handle(hbt_in);
    if (!hbt || !hbt->huart) return BT_INVALID;

    /* EN 输出：默认低（透传模式） */
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = hbt->EN_Pin;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(hbt->EN_Port, &gpio);
    HAL_GPIO_WritePin(hbt->EN_Port, hbt->EN_Pin, GPIO_PIN_RESET);

    /* STATE 输入 */
    gpio.Pin  = hbt->STATE_Pin;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(hbt->STATE_Port, &gpio);

    hbt->initialized = 1;
    return BT_OK;
}

BT_Status_t BT_DeInit(BT_Handle_t *hbt_in)
{
    BT_Handle_t *hbt = _handle(hbt_in);
    if (!hbt) return BT_INVALID;
    hbt->initialized = 0;
    return BT_OK;
}

BT_Status_t BT_Send(BT_Handle_t *hbt_in, const uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
    BT_Handle_t *hbt = _handle(hbt_in);
    if (!hbt || !hbt->initialized || !data || !len) return BT_INVALID;

    /* 仅在透传模式 (EN 低) 允许发送 */
    if (HAL_GPIO_ReadPin(hbt->EN_Port, hbt->EN_Pin) == GPIO_PIN_SET)
        return BT_ERROR;

    if (UART_SendEx(hbt->huart, data, len, timeout_ms) != HAL_OK)
        return BT_ERROR;
    return BT_OK;
}

BT_Status_t BT_Receive(BT_Handle_t *hbt_in, uint8_t *buf, uint16_t len, uint32_t timeout_ms)
{
    BT_Handle_t *hbt = _handle(hbt_in);
    if (!hbt || !hbt->initialized || !buf || !len) return BT_INVALID;

    if (UART_ReceiveEx(hbt->huart, buf, len, timeout_ms) != HAL_OK)
        return BT_TIMEOUT;
    return BT_OK;
}

/* 等待 AT 响应：简单按字节读取到 '\n' 或满缓冲/超时 */
static BT_Status_t _wait_resp(BT_Handle_t *hbt, char *resp, uint16_t resp_len, uint32_t timeout_ms)
{
    if (!resp || resp_len < 2) {
        /* 若不需要响应内容，只延时等待 */
        HAL_Delay(timeout_ms);
        return BT_OK;
    }

    uint32_t start = HAL_GetTick();
    uint16_t idx = 0;
    while ((HAL_GetTick() - start) < timeout_ms) {
        uint8_t ch;
        if (UART_ReceiveEx(hbt->huart, &ch, 1, 5) == HAL_OK) {
            if (idx < (resp_len - 1)) {
                resp[idx++] = (char)ch;
            }
            if (ch == '\n') break; /* 行结束 */
        }
    }
    resp[idx] = '\0';
    return (idx == 0) ? BT_TIMEOUT : BT_OK;
}

BT_Status_t BT_SendAT(BT_Handle_t *hbt_in, const char *cmd,
                      char *resp_buf, uint16_t resp_len, uint32_t timeout_ms)
{
    BT_Handle_t *hbt = _handle(hbt_in);
    if (!hbt || !hbt->initialized || !cmd) return BT_INVALID;

    /* 进入 AT 模式：拉高 EN（模块会断开连接进入指令模式） */
    while (HAL_GPIO_ReadPin(hbt->EN_Port, hbt->EN_Pin) == GPIO_PIN_RESET)
    {
        HAL_GPIO_WritePin(hbt->EN_Port, hbt->EN_Pin, GPIO_PIN_SET);
    }


    /* 直接阻塞发送 AT 命令（保证顺序） */
    if (HAL_UART_Transmit(hbt->huart, (uint8_t*)cmd, (uint16_t)strlen(cmd), 1000) != HAL_OK) {
        HAL_GPIO_WritePin(hbt->EN_Port, hbt->EN_Pin, GPIO_PIN_SET);
        return BT_ERROR;
    }

    /* 等待响应 */
    BT_Status_t st = _wait_resp(hbt, resp_buf, resp_len, timeout_ms);

    /* 退出 AT 模式，恢复透传 */
    HAL_GPIO_WritePin(hbt->EN_Port, hbt->EN_Pin, GPIO_PIN_RESET);
    HAL_Delay(20);

    return st;
}

BT_ConnState_t BT_GetState(BT_Handle_t *hbt_in)
{
    BT_Handle_t *hbt = _handle(hbt_in);
    if (!hbt || !hbt->initialized) return BT_DISCONNECTED;
    return (HAL_GPIO_ReadPin(hbt->STATE_Port, hbt->STATE_Pin) == GPIO_PIN_SET)
            ? BT_CONNECTED : BT_DISCONNECTED;
}