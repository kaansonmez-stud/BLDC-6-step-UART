#pragma once

#include "gd32f1x0.h"
#include <stdint.h>

/*
 * PWM (TIMER0 CH0/1/2, PA8/PA9/PA10) ve LIN (PB13/PB14/PB15) sürücü
 * GPIO + Timer init, PWM/LIN kontrol fonksiyonları
 */

void pwm_lin_init(uint32_t pwm_hz); /* GPIO + Timer0 init */

/* PWM kontrol */
void pwm_set(uint32_t ch, uint16_t ccr);
void pwm_enable(uint32_t ch, uint8_t en);
void pwm_all_off(void);

/* LIN kontrol */
void lin_write(uint16_t pin, uint8_t on);
void lin_all_off(void);
void lin_all_on(void);

/* CCR hesaplama */
uint16_t drv_to_ccr(uint8_t drv);
uint16_t duty_to_ccr(uint8_t duty_percent);

/* Pin tanımları (diğer modüller için) */
#define LIN_U_PIN GPIO_PIN_13
#define LIN_V_PIN GPIO_PIN_14
#define LIN_W_PIN GPIO_PIN_15
