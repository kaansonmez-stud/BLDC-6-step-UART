#pragma once

#include <stdint.h>

/* GD32F130C8T6 — USART1, PA2 TX, PA3 RX, AF1, 115200 8N1
 * Interrupt-driven RX (karakter kaybını önler)
 */

void uart_init(void);
void uart_putc(uint8_t c);
void uart_print(const char *s);
void uart_print_int(int32_t v);
uint8_t uart_rx_ready(void);
uint8_t uart_getc(void);
