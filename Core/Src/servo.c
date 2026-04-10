/**
 * @file    servo.c
 * @brief   SG90 Servo Motor Driver — Hardware PWM via TIM2 CH1 (PA0)
 *
 *          Timer configuration (set in MX_TIM2_Init):
 *            Prescaler = 71  -> Timer clock = 72MHz / 72 = 1 MHz (1 tick = 1 us)
 *            Period    = 19999 -> PWM period = 20 000 us = 20 ms = 50 Hz
 *
 *          Pulse width mapping (SG90 datasheet):
 *            0   deg -> 500  us (0.5 ms)
 *            90  deg -> 1500 us (1.5 ms)
 *            180 deg -> 2500 us (2.5 ms)
 */
#include "servo.h"

static TIM_HandleTypeDef *_htim = NULL;

/**
 * @brief  Initialise the servo: start hardware PWM output on TIM2 CH1.
 * @param  htim  Pointer to the TIM2 handle initialised by MX_TIM2_Init().
 */
void Servo_Init(TIM_HandleTypeDef *htim)
{
    _htim = htim;
    /* Start hardware PWM — signal appears immediately on PA0 */
    HAL_TIM_PWM_Start(_htim, SERVO_CHANNEL);
    /* Move to default centre position (90 degrees) */
    Servo_SetAngle(90);
}

/**
 * @brief  Move the servo to the specified angle.
 * @param  angle  Desired position in degrees (0 – 180).
 *                Values outside this range are clamped automatically.
 */
void Servo_SetAngle(uint8_t angle)
{
    if (angle > 180) angle = 180;

    /* Linear interpolation between SERVO_MIN_PULSE and SERVO_MAX_PULSE */
    uint32_t pulse = SERVO_MIN_PULSE +
        ((uint32_t)angle * (SERVO_MAX_PULSE - SERVO_MIN_PULSE)) / 180U;

    __HAL_TIM_SET_COMPARE(_htim, SERVO_CHANNEL, pulse);
}
