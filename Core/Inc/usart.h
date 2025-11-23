/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;

extern UART_HandleTypeDef huart2;

extern UART_HandleTypeDef huart3;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);
void MX_USART3_UART_Init(void);

/* USER CODE BEGIN Prototypes */

/* 简单的阻塞串口发送/接收辅助函数 */
/* 传入任意串口句柄的通用版本 */
/* 通过 HAL_StatusTypeDef 返回状态，timeout 单位 ms */
HAL_StatusTypeDef UART_SendEx(UART_HandleTypeDef *huart, const uint8_t *data, uint16_t len, uint32_t timeout);
HAL_StatusTypeDef UART_SendStringEx(UART_HandleTypeDef *huart, const char *s);/* 发送以 '\\0' 结尾的字符串 */
HAL_StatusTypeDef UART_PrintfEx(UART_HandleTypeDef *huart, const char *fmt, ...);/* printf 风格，内部缓冲区 256 字节 */
HAL_StatusTypeDef UART_ReceiveEx(UART_HandleTypeDef *huart, uint8_t *buf, uint16_t len, uint32_t timeout);
HAL_StatusTypeDef UART_ReceiveCharEx(UART_HandleTypeDef *huart, uint8_t *ch, uint32_t timeout);/* 接收单字符 */

/*** 通用中断收发支持：USART1 / USART2 / USART3 ***/
#define UART_IT_RX_BUF_SIZE 256

/* 每个串口一套状态变量 */

/* USART1 */
extern volatile uint8_t  uart1_tx_done;
extern volatile uint8_t  uart1_rx_done;
extern uint8_t           uart1_rx_buf[UART_IT_RX_BUF_SIZE];
extern uint16_t          uart1_rx_len;

/* USART2 */
extern volatile uint8_t  uart2_tx_done;
extern volatile uint8_t  uart2_rx_done;
extern uint8_t           uart2_rx_buf[UART_IT_RX_BUF_SIZE];
extern uint16_t          uart2_rx_len;

/* USART3 */
extern volatile uint8_t  uart3_tx_done;
extern volatile uint8_t  uart3_rx_done;
extern uint8_t           uart3_rx_buf[UART_IT_RX_BUF_SIZE];
extern uint16_t          uart3_rx_len;

/* 启动一次中断发送（传入任意 UART） */
HAL_StatusTypeDef UART_IT_Send(UART_HandleTypeDef *huart, const uint8_t *data, uint16_t len);

/* 启动一次中断接收到固定长度（传入任意 UART） */
HAL_StatusTypeDef UART_IT_StartRecv(UART_HandleTypeDef *huart, uint16_t expect_len);
// 启动：UART_IT_StartRecv(&huart1, 20);
// 轮询：if (uart1_rx_done) { 使用 uart1_rx_buf / uart1_rx_len }

/// 或 ///

// 启动：UART_IT_StartRecv(&huart3, 50);
// 轮询：if (uart3_rx_done) { 使用 uart3_rx_buf / uart3_rx_len }

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

