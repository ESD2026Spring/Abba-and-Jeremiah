/**
 * @file    lcd_i2c.h
 * @brief   HD44780 16x2 LCD Driver via PCF8574 I2C Expander
 *          I2C1: SCL=PB8, SDA=PB9
 *          Default I2C address: 0x27 (change to 0x3F if LCD shows nothing)
 */
#ifndef INC_LCD_I2C_H_
#define INC_LCD_I2C_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* PCF8574 I2C address (7-bit shifted left by 1 for HAL) */
/* Try 0x3F if 0x27 does not work */
#define LCD_I2C_ADDR    (0x27 << 1)

#define LCD_COLS        16
#define LCD_ROWS        2

void LCD_Init(I2C_HandleTypeDef *hi2c);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t col, uint8_t row);
void LCD_WriteChar(char c);
void LCD_WriteString(const char *str);

#endif /* INC_LCD_I2C_H_ */
