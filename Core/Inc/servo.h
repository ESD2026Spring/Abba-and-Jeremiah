/**
 * @file    servo.h
 * @brief   SG90 Servo Motor Driver using TIM2 CH1 Hardware PWM
 *          Signal Pin: PA0 (TIM2_CH1)
 *          PWM Period : 20ms (50Hz)
 *          Angle range: 0 to 180 degrees
 */
#ifndef INC_SERVO_H_
#define INC_SERVO_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* Timer channel used for servo PWM output */
#define SERVO_CHANNEL     TIM_CHANNEL_1

/*
 * SG90 pulse widths (in timer ticks at 1 MHz = 1 tick per microsecond)
 * Prescaler=71, ARR=19999 => 1MHz tick, 20ms period
 *   0 deg  => 500  us
 * 180 deg  => 2500 us
 */
#define SERVO_MIN_PULSE   500
#define SERVO_MAX_PULSE   2500

void Servo_Init(TIM_HandleTypeDef *htim);
void Servo_SetAngle(uint8_t angle);

#endif /* INC_SERVO_H_ */
