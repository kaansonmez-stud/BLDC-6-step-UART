#pragma once

#include <stdint.h>

/* SysTick tabanlı 1ms zaman tabanı */

extern volatile uint32_t g_ms;

void timebase_init(void); /* SysTick_Config çağırır */
void delay_ms(uint32_t ms);
