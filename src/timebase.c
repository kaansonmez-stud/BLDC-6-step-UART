#include "timebase.h"
#include "gd32f1x0.h"


volatile uint32_t g_ms = 0;

void SysTick_Handler(void) { g_ms++; }

void timebase_init(void) { SysTick_Config(SystemCoreClock / 1000U); }

void delay_ms(uint32_t ms) {
  uint32_t t0 = g_ms;
  while ((g_ms - t0) < ms) {
    __NOP();
  }
}
