#ifndef __SSD1306_H__
#define __SSD1306_H__

#include "stm32f4xx_hal.h"
#include <stdint.h>

void ssd1306_Init(void);
void ssd1306_Fill(uint8_t color);
void ssd1306_UpdateScreen(void);
void ssd1306_SetCursor(uint8_t x, uint8_t y);
void ssd1306_WriteString(uint8_t x, uint8_t y, const char *str);
void ssd1306_WriteInt(uint8_t x, uint8_t y, int val);

#endif /* __SSD1306_H__ */
