#include "pwm_lin.h"
#include "gd32f1x0.h"
#include "hall_pins.h"

/* ====================== Ayarlar ====================== */
#define PWM_TIMER TIMER0
#define LOW_ACTIVE_HIGH 0

#define LIN_PORT GPIOB

static uint16_t g_arr = 0;

/* ====================== LIN ====================== */
void lin_write(uint16_t pin, uint8_t on) {
#if LOW_ACTIVE_HIGH
  on ? gpio_bit_set(LIN_PORT, pin) : gpio_bit_reset(LIN_PORT, pin);
#else
  on ? gpio_bit_reset(LIN_PORT, pin) : gpio_bit_set(LIN_PORT, pin);
#endif
}

void lin_all_off(void) {
  lin_write(LIN_U_PIN, 0);
  lin_write(LIN_V_PIN, 0);
  lin_write(LIN_W_PIN, 0);
}

void lin_all_on(void) {
  lin_write(LIN_U_PIN, 1);
  lin_write(LIN_V_PIN, 1);
  lin_write(LIN_W_PIN, 1);
}

/* ====================== PWM ====================== */
void pwm_set(uint32_t ch, uint16_t ccr) {
  timer_channel_output_pulse_value_config(PWM_TIMER, ch, ccr);
}

void pwm_enable(uint32_t ch, uint8_t en) {
  timer_channel_output_state_config(PWM_TIMER, ch,
                                    en ? TIMER_CCX_ENABLE : TIMER_CCX_DISABLE);
}

void pwm_all_off(void) {
  pwm_set(TIMER_CH_0, 0);
  pwm_enable(TIMER_CH_0, 0);
  pwm_set(TIMER_CH_1, 0);
  pwm_enable(TIMER_CH_1, 0);
  pwm_set(TIMER_CH_2, 0);
  pwm_enable(TIMER_CH_2, 0);
}

/* ====================== CCR hesaplama ====================== */
uint16_t drv_to_ccr(uint8_t drv) {
  if (drv == 0)
    return 0;
  uint32_t ccr = ((uint32_t)(g_arr + 1U) * drv) / 256U;
  if (ccr > g_arr)
    ccr = g_arr;
  return (uint16_t)ccr;
}

uint16_t duty_to_ccr(uint8_t duty_percent) {
  if (duty_percent == 0)
    return 0;
  if (duty_percent > 95)
    duty_percent = 95;
  uint32_t ccr = ((uint32_t)(g_arr + 1U) * duty_percent) / 100U;
  if (ccr > g_arr)
    ccr = g_arr;
  return (uint16_t)ccr;
}

/* ====================== Init ====================== */
void pwm_lin_init(uint32_t pwm_hz) {
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_GPIOB);
  rcu_periph_clock_enable(RCU_GPIOC);

  // PWM pins: PA8/PA9/PA10 -> AF2 TIMER0 CH0/1/2
  gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,
                GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);
  gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                          GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);
  gpio_af_set(GPIOA, GPIO_AF_2, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);

  // LIN pins: PB13/14/15 GPIO output
  gpio_mode_set(LIN_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                LIN_U_PIN | LIN_V_PIN | LIN_W_PIN);
  gpio_output_options_set(LIN_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                          LIN_U_PIN | LIN_V_PIN | LIN_W_PIN);
  lin_all_off();

  // Hall pins: input pull-up
  gpio_mode_set(HALLA_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, HALLA_PIN);
  gpio_mode_set(HALLB_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, HALLB_PIN);
  gpio_mode_set(HALLC_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, HALLC_PIN);

  // TIMER0 PWM
  rcu_periph_clock_enable(RCU_TIMER0);
  timer_deinit(PWM_TIMER);

  timer_parameter_struct t;
  timer_struct_para_init(&t);

  uint32_t clk = SystemCoreClock;
  uint32_t period = clk / pwm_hz;
  if (period < 200)
    period = 200;
  period -= 1;
  if (period > 0xFFFF)
    period = 0xFFFF;
  g_arr = (uint16_t)period;

  t.prescaler = 0;
  t.alignedmode = TIMER_COUNTER_EDGE;
  t.counterdirection = TIMER_COUNTER_UP;
  t.period = g_arr;
  t.clockdivision = TIMER_CKDIV_DIV1;
  t.repetitioncounter = 0;
  timer_init(PWM_TIMER, &t);

  timer_oc_parameter_struct oc;
  timer_channel_output_struct_para_init(&oc);
  oc.outputstate = TIMER_CCX_ENABLE;
  oc.ocpolarity = TIMER_OC_POLARITY_HIGH;
  oc.ocidlestate = TIMER_OC_IDLE_STATE_LOW;

  timer_channel_output_config(PWM_TIMER, TIMER_CH_0, &oc);
  timer_channel_output_mode_config(PWM_TIMER, TIMER_CH_0, TIMER_OC_MODE_PWM0);
  timer_channel_output_shadow_config(PWM_TIMER, TIMER_CH_0,
                                     TIMER_OC_SHADOW_DISABLE);

  timer_channel_output_config(PWM_TIMER, TIMER_CH_1, &oc);
  timer_channel_output_mode_config(PWM_TIMER, TIMER_CH_1, TIMER_OC_MODE_PWM0);
  timer_channel_output_shadow_config(PWM_TIMER, TIMER_CH_1,
                                     TIMER_OC_SHADOW_DISABLE);

  timer_channel_output_config(PWM_TIMER, TIMER_CH_2, &oc);
  timer_channel_output_mode_config(PWM_TIMER, TIMER_CH_2, TIMER_OC_MODE_PWM0);
  timer_channel_output_shadow_config(PWM_TIMER, TIMER_CH_2,
                                     TIMER_OC_SHADOW_DISABLE);

  timer_primary_output_config(PWM_TIMER, ENABLE);
  timer_enable(PWM_TIMER);

  pwm_all_off();
}
