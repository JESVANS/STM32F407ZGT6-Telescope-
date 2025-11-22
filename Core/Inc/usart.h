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

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

