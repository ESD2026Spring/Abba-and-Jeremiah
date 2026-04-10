/**
 * @file    keypad.h
 * @brief   4x4 Matrix Keypad Driver Header
 *          Rows: PB0-PB3 (OUTPUT)
 *          Cols: PB4-PB7 (INPUT, Pull-Up)
 *          '*' = delete last digit, '#' = confirm entry
 */
#ifndef INC_KEYPAD_H_
#define INC_KEYPAD_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>

#define KEYPAD_ROWS   4
#define KEYPAD_COLS   4
#define NO_KEY        '\0'

/* Row pins: PB0-PB3 (OUTPUT) */
#define ROW_PORT      GPIOB
#define ROW_PIN_0     GPIO_PIN_0
#define ROW_PIN_1     GPIO_PIN_1
#define ROW_PIN_2     GPIO_PIN_2
#define ROW_PIN_3     GPIO_PIN_3

/* Col pins: PB4-PB7 (INPUT Pull-Up) */
#define COL_PORT      GPIOB
#define COL_PIN_0     GPIO_PIN_4
#define COL_PIN_1     GPIO_PIN_5
#define COL_PIN_2     GPIO_PIN_6
#define COL_PIN_3     GPIO_PIN_7

void Keypad_Init(void);
char Keypad_GetKey(void);

#endif /* INC_KEYPAD_H_ */
