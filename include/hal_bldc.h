#pragma once

#include <stdint.h>

/*
 * BLDC 6-Step Hall Commutation
 * Hall: PB11 (A), PA1 (B), PC14 (C)
 *
 * UYARI: PC14, LSE oscillator pini ile paylaşımlıdır.
 * Kartında LSE kristali/RTC kullanılıyorsa Hall C hatalı okunabilir.
 */

uint8_t hall_read_abc(void);
uint8_t hall_valid(uint8_t h);
int8_t hall_to_sector(uint8_t hall);

void apply_sector(uint8_t sector, uint16_t pwm_ccr);
void commutate_from_hall(uint8_t hall, uint16_t pwm_ccr, uint8_t dir_fwd);
void kick_start(uint16_t pwm_ccr, uint8_t dir_fwd);

/* Runtime sector shift ayarı (-5..+5) */
void bldc_set_shift(int8_t s);
int8_t bldc_get_shift(void);

/* Otomatik hall-sector eşleme tespiti */
void bldc_detect(uint16_t pwm_ccr, uint8_t det[6]);
