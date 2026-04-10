/**
 * @file    keypad.c
 * @brief   4x4 Matrix Keypad Driver Implementation
 *          Scans rows LOW one at a time and reads columns.
 *          Includes 20ms debounce and waits for key release.
 *
 *          Key map:
 *           1 2 3 A
 *           4 5 6 B
 *           7 8 9 C
 *           * 0 # D
 *
 *          '*' = delete last digit
 *          '#' = confirm/enter
 */
#include "keypad.h"
#include "cmsis_os.h"

/* Key map matching physical layout */
static const char keyMap[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

static const uint16_t rowPins[KEYPAD_ROWS] = {
    ROW_PIN_0, ROW_PIN_1, ROW_PIN_2, ROW_PIN_3
};

static const uint16_t colPins[KEYPAD_COLS] = {
    COL_PIN_0, COL_PIN_1, COL_PIN_2, COL_PIN_3
};

/**
 * @brief  Initialise keypad GPIO.
 *         Drives all row pins HIGH (idle state).
 *         Column pins must be configured as Input Pull-Up in CubeMX.
 */
void Keypad_Init(void)
{
    HAL_GPIO_WritePin(ROW_PORT,
        ROW_PIN_0 | ROW_PIN_1 | ROW_PIN_2 | ROW_PIN_3,
        GPIO_PIN_SET);
}

/**
 * @brief  Scan all rows and columns for a pressed key.
 * @retval Pressed key character, or NO_KEY ('\0') if nothing pressed.
 */
char Keypad_GetKey(void)
{
    for (uint8_t row = 0; row < KEYPAD_ROWS; row++)
    {
        /* Pull current row LOW, all others HIGH */
        HAL_GPIO_WritePin(ROW_PORT,
            ROW_PIN_0 | ROW_PIN_1 | ROW_PIN_2 | ROW_PIN_3,
            GPIO_PIN_SET);
        HAL_GPIO_WritePin(ROW_PORT, rowPins[row], GPIO_PIN_RESET);

        osDelay(2); /* settle time */

        for (uint8_t col = 0; col < KEYPAD_COLS; col++)
        {
            if (HAL_GPIO_ReadPin(COL_PORT, colPins[col]) == GPIO_PIN_RESET)
            {
                osDelay(20); /* debounce */

                if (HAL_GPIO_ReadPin(COL_PORT, colPins[col]) == GPIO_PIN_RESET)
                {
                    /* Wait for key release before returning */
                    while (HAL_GPIO_ReadPin(COL_PORT, colPins[col]) == GPIO_PIN_RESET)
                        osDelay(5);

                    return keyMap[row][col];
                }
            }
        }
    }

    return NO_KEY;
}
