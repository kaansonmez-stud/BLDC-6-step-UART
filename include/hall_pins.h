#pragma once

#include "gd32f1x0.h"

/*
 * Hall sensör pin tanımları — tek kaynak (single source of truth)
 *
 * UYARI: PC14, LSE oscillator pini ile paylaşımlıdır.
 * Kartında LSE kristali/RTC kullanılıyorsa Hall C hatalı okunabilir.
 */
#define HALLA_PORT GPIOB
#define HALLA_PIN GPIO_PIN_11
#define HALLB_PORT GPIOA
#define HALLB_PIN GPIO_PIN_1
#define HALLC_PORT GPIOC
#define HALLC_PIN GPIO_PIN_14
