#include "hal_bldc.h"
#include "gd32f1x0.h"
#include "hall_pins.h"
#include "pwm_lin.h"
#include "timebase.h"

static int8_t g_sector_shift = -1; /* UART'tan ayarlanabilir */

void bldc_set_shift(int8_t s) { g_sector_shift = s; }
int8_t bldc_get_shift(void) { return g_sector_shift; }
#define KICK_STEPS 120U
#define KICK_DELAY_MS0 10U
#define KICK_DELAY_MS1 3U

/* ====================== Hall read ====================== */
uint8_t hall_read_abc(void) {
  // bit0=A, bit1=B, bit2=C
  uint8_t a = gpio_input_bit_get(HALLA_PORT, HALLA_PIN) ? 1U : 0U;
  uint8_t b = gpio_input_bit_get(HALLB_PORT, HALLB_PIN) ? 1U : 0U;
  uint8_t c = gpio_input_bit_get(HALLC_PORT, HALLC_PIN) ? 1U : 0U;
  return (uint8_t)((a << 0) | (b << 1) | (c << 2));
}

uint8_t hall_valid(uint8_t h) { return (h != 0x0 && h != 0x7); }

/* Hall -> sector (0..5) mapping (tipik 120°) */
int8_t hall_to_sector(uint8_t hall) {
  switch (hall) {
  case 0x1:
    return 0; // 001
  case 0x5:
    return 1; // 101
  case 0x4:
    return 2; // 100
  case 0x6:
    return 3; // 110
  case 0x2:
    return 4; // 010
  case 0x3:
    return 5; // 011
  default:
    return -1;
  }
}

/* ====================== Commutation ====================== */
/* Sector tablosu:
   0: U+ V-    1: U+ W-    2: V+ W-
   3: V+ U-    4: W+ U-    5: W+ V-
*/
void apply_sector(uint8_t sector, uint16_t pwm_ccr) {
  static const uint8_t plus_ch[6] = {TIMER_CH_0, TIMER_CH_0, TIMER_CH_1,
                                     TIMER_CH_1, TIMER_CH_2, TIMER_CH_2};
  static const uint16_t minus_lin[6] = {LIN_V_PIN, LIN_W_PIN, LIN_W_PIN,
                                        LIN_U_PIN, LIN_U_PIN, LIN_V_PIN};

  pwm_all_off();
  lin_all_off();

  pwm_set(plus_ch[sector], pwm_ccr);
  pwm_enable(plus_ch[sector], 1);
  lin_write(minus_lin[sector], 1);
}

void commutate_from_hall(uint8_t hall, uint16_t pwm_ccr, uint8_t dir_fwd) {
  int8_t s = hall_to_sector(hall);
  if (s < 0) {
    pwm_all_off();
    lin_all_off();
    return;
  }

  int8_t sector = s;

  if (!dir_fwd)
    sector = (int8_t)((sector + 3) % 6);

  sector = (int8_t)(sector + g_sector_shift);
  sector %= 6;
  if (sector < 0)
    sector += 6;

  apply_sector((uint8_t)sector, pwm_ccr);
}

/* Kick: hall beklemeden sektörleri dolaştırıp rotorun kalkmasını sağlar */
void kick_start(uint16_t pwm_ccr, uint8_t dir_fwd) {
  for (uint32_t i = 0; i < KICK_STEPS; i++) {
    uint32_t di = KICK_DELAY_MS0;
    if (KICK_STEPS > 1) {
      di = KICK_DELAY_MS0 -
           ((KICK_DELAY_MS0 - KICK_DELAY_MS1) * i) / (KICK_STEPS - 1);
    }

    uint8_t sec = (uint8_t)(i % 6);

    if (!dir_fwd)
      sec = (uint8_t)((sec + 3) % 6);

    int8_t sec2 = (int8_t)sec + g_sector_shift;
    sec2 %= 6;
    if (sec2 < 0)
      sec2 += 6;

    apply_sector((uint8_t)sec2, pwm_ccr);
    delay_ms(di);
  }
}

/* Detect: her sektörü uygula, rotoru yerleştir, hall oku */
void bldc_detect(uint16_t pwm_ccr, uint8_t det[6]) {
  for (uint8_t s = 0; s < 6; s++) {
    apply_sector(s, pwm_ccr);
    delay_ms(300);
    det[s] = hall_read_abc();
  }
  pwm_all_off();
  lin_all_off();
}
