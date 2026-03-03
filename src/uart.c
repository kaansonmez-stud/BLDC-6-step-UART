#include "uart.h"
#include "gd32f1x0.h"

/* ---- RX Ring Buffer (interrupt ile doldurulur) ---- */
#define RX_BUF_SIZE 64
static volatile uint8_t rx_buf[RX_BUF_SIZE];
static volatile uint8_t rx_head = 0; /* ISR yazar */
static volatile uint8_t rx_tail = 0; /* main okur */

/* USART1 interrupt handler */
void USART1_IRQHandler(void) {
  /* ORE/FE/NE hata bayraklarını temizle — yoksa RX takılabilir */
  if (usart_flag_get(USART1, USART_FLAG_ORERR) != RESET) {
    usart_flag_clear(USART1, USART_FLAG_ORERR);
  }
  if (usart_flag_get(USART1, USART_FLAG_FERR) != RESET) {
    usart_flag_clear(USART1, USART_FLAG_FERR);
  }
  if (usart_flag_get(USART1, USART_FLAG_NERR) != RESET) {
    usart_flag_clear(USART1, USART_FLAG_NERR);
  }

  if (usart_interrupt_flag_get(USART1, USART_INT_FLAG_RBNE) != RESET) {
    uint8_t c = (uint8_t)(usart_data_receive(USART1) & 0xFFu);
    uint8_t next = (rx_head + 1) % RX_BUF_SIZE;
    if (next != rx_tail) { /* buffer dolu değilse kaydet */
      rx_buf[rx_head] = c;
      rx_head = next;
    }
  }
}

void uart_init(void) {
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_USART1);

  /* PA2 TX, PA3 RX -> AF1 */
  gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_2 | GPIO_PIN_3);

  gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
  gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
  gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_3);

  usart_deinit(USART1);
  usart_baudrate_set(USART1, 115200U);
  usart_word_length_set(USART1, USART_WL_8BIT);
  usart_stop_bit_set(USART1, USART_STB_1BIT);
  usart_parity_config(USART1, USART_PM_NONE);
  usart_hardware_flow_rts_config(USART1, USART_RTS_DISABLE);
  usart_hardware_flow_cts_config(USART1, USART_CTS_DISABLE);
  usart_receive_config(USART1, USART_RECEIVE_ENABLE);
  usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);

  /* RX interrupt aktifleştir */
  usart_interrupt_enable(USART1, USART_INT_RBNE);
  nvic_irq_enable(USART1_IRQn, 1, 0);

  usart_enable(USART1);
}

void uart_putc(uint8_t c) {
  while (RESET == usart_flag_get(USART1, USART_FLAG_TBE)) {
  }
  usart_data_transmit(USART1, c);
}

void uart_print(const char *s) {
  while (*s)
    uart_putc((uint8_t)*s++);
}

void uart_print_int(int32_t v) {
  char buf[12];
  int i = 0;
  if (v < 0) {
    uart_putc('-');
    v = -v;
  }
  if (v == 0) {
    uart_putc('0');
    return;
  }
  while (v) {
    buf[i++] = '0' + (v % 10);
    v /= 10;
  }
  while (i--)
    uart_putc((uint8_t)buf[i]);
}

uint8_t uart_rx_ready(void) { return (rx_head != rx_tail); }

uint8_t uart_getc(void) {
  while (rx_head == rx_tail) {
  } /* bekle (normalde rx_ready kontrolünden sonra çağrılır) */
  uint8_t c = rx_buf[rx_tail];
  rx_tail = (rx_tail + 1) % RX_BUF_SIZE;
  return c;
}
