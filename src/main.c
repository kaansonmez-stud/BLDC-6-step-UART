/*
 * GD32F130C8T6 — BLDC Motor + UART Kontrol (Modüler)
 *
 * Modüller:
 *   timebase.h/.c  -> SysTick, g_ms, delay_ms
 *   uart.h/.c      -> USART1 PA2/PA3 AF1 115200
 *   cmd.h/.c       -> Komut parser (DRV, DUTY, DIR, STOP, BRAKE)
 *   pwm_lin.h/.c   -> GPIO, Timer0 PWM, LIN kontrol
 *   hal_bldc.h/.c  -> Hall commutation, kick-start
 *   main.c         -> Bu dosya: her şeyi birleştirir
 */
#include "cmd.h"
#include "gd32f1x0.h"
#include "hal_bldc.h"
#include "pwm_lin.h"
#include "timebase.h"
#include "uart.h"

/* ====================== Ayarlar ====================== */
#define PWM_FREQ_HZ 32000U
#define BOOT_SETTLE_MS 250U
#define STALL_TIMEOUT_MS 350U
#define DBG_INTERVAL_MS 500U /* Debug print aralığı */

/* ====================== Runtime state ====================== */
static uint8_t g_drv = 0;         /* 0..255 */
static uint8_t g_duty = 0;        /* 0..100 */
static uint8_t g_dir_forward = 1; /* 1=ileri, 0=geri */
static uint8_t g_running = 0;
static uint8_t g_braking = 0;
static uint32_t g_dbg_ms = 0; /* Son debug print zamanı */

/* Aktif CCR: DRV > 0 ise DRV, yoksa DUTY */
static inline uint16_t get_active_ccr(void) {
  if (g_drv > 0)
    return drv_to_ccr(g_drv);
  return duty_to_ccr(g_duty);
}

/* ====================== Main ====================== */
int main(void) {
  SystemCoreClockUpdate();
  timebase_init();
  uart_init();
  pwm_lin_init(PWM_FREQ_HZ);

  pwm_all_off();
  lin_all_off();
  delay_ms(BOOT_SETTLE_MS);

  uart_print("BOOT\n");
  uart_print("Komutlar: DRV DUTY DIR STOP BRAKE\n");

  cmd_state_t cmd;
  cmd_init(&cmd);

  uint8_t last_hall = hall_read_abc();
  uint32_t last_change_ms = g_ms;

  while (1) {
    /* ---- UART komut polling ---- */
    cmd_result_t r = cmd_poll(&cmd, g_ms);

    switch (r.type) {
    case CMD_DRV:
      g_drv = (uint8_t)r.value;
      g_duty = 0; /* DRV aktif → DUTY devre dışı */
      g_braking = 0;
      if (g_drv > 0 && !g_running) {
        g_running = 1;
        /* Motor başlatılıyor: hemen ilk commutation yap */
        last_hall = hall_read_abc();
        uint16_t start_ccr = drv_to_ccr(g_drv);
        if (!hall_valid(last_hall)) {
          kick_start(start_ccr, g_dir_forward);
          last_hall = hall_read_abc();
        }
        commutate_from_hall(last_hall, start_ccr, g_dir_forward);
        last_change_ms = g_ms;
      }
      g_running = (g_drv > 0 || g_duty > 0) ? 1 : 0;
      /* DEBUG */ uart_print("DBG DRV: hall=");
      uart_print_int(hall_read_abc());
      uart_print(" ccr=");
      uart_print_int(drv_to_ccr(g_drv));
      uart_print(" run=");
      uart_print_int(g_running);
      uart_print("\n");
      break;

    case CMD_DUTY:
      g_duty = (uint8_t)r.value;
      g_drv = 0; /* DUTY aktif → DRV devre dışı */
      g_braking = 0;
      if (g_duty > 0 && !g_running) {
        g_running = 1;
        /* Motor başlatılıyor: hemen ilk commutation yap */
        last_hall = hall_read_abc();
        uint16_t start_ccr2 = duty_to_ccr(g_duty);
        if (!hall_valid(last_hall)) {
          kick_start(start_ccr2, g_dir_forward);
          last_hall = hall_read_abc();
        }
        commutate_from_hall(last_hall, start_ccr2, g_dir_forward);
        last_change_ms = g_ms;
      }
      g_running = (g_duty > 0 || g_drv > 0) ? 1 : 0;
      /* DEBUG */ uart_print("DBG DUTY: hall=");
      uart_print_int(hall_read_abc());
      uart_print(" ccr=");
      uart_print_int(duty_to_ccr(g_duty));
      uart_print(" run=");
      uart_print_int(g_running);
      uart_print("\n");
      break;

    case CMD_DIR:
      g_dir_forward = (uint8_t)r.value;
      break;

    case CMD_STOP:
      g_drv = 0;
      g_duty = 0;
      g_running = 0;
      g_braking = 0;
      pwm_all_off();
      lin_all_off();
      break;

    case CMD_BRAKE:
      g_drv = 0;
      g_duty = 0;
      g_running = 0;
      g_braking = 1;
      pwm_all_off();
      lin_all_on(); /* 3 low-side ON = kısa devre frenleme */
      break;
    
    case CMD_SHIFT:
      bldc_set_shift((int8_t)r.value);
      
    case CMD_NONE:
    default:
      break;

    case CMD_DETECT: {
      g_drv = 0;
      g_duty = 0;
      g_running = 0;
      g_braking = 0;
      uint8_t det[6];
      bldc_detect(drv_to_ccr(80), det);
      uart_print("DETECT sonuc:\n");
      for (uint8_t i = 0; i < 6; i++) {
        uart_print("  S");
        uart_print_int(i);
        uart_print(" -> hall=");
        uart_print_int(det[i]);
        uart_print("\n");
      }
      uart_print("DETECT bitti\n");
      break;
    }
    }

    /* ---- Fren modunda commutation yapma ---- */
    if (g_braking)
      continue;

    /* ---- Motor commutation ---- */
    uint16_t ccr = get_active_ccr();

    if (ccr == 0 || !g_running) {
      pwm_all_off();
      lin_all_off();
      last_hall = hall_read_abc();
      last_change_ms = g_ms;
      continue;
    }

    /* DEBUG: periyodik durum bilgisi */
    if ((g_ms - g_dbg_ms) >= DBG_INTERVAL_MS) {
      g_dbg_ms = g_ms;
      uint8_t dh = hall_read_abc();
      uart_print("DBG: h=");
      uart_print_int(dh);
      uart_print(" val=");
      uart_print_int(hall_valid(dh));
      uart_print(" ccr=");
      uart_print_int(ccr);
      uart_print(" run=");
      uart_print_int(g_running);
      uart_print("\n");
    }

    uint8_t h = hall_read_abc();

    if (hall_valid(h) && h != last_hall) {
      last_hall = h;
      last_change_ms = g_ms;
      commutate_from_hall(last_hall, ccr, g_dir_forward);
    } else if ((g_ms - last_change_ms) > STALL_TIMEOUT_MS) {
      /* DEBUG */ uart_print("DBG STALL: kick h=");
      uart_print_int(h);
      uart_print("\n");
      kick_start(ccr, g_dir_forward);
      last_hall = hall_read_abc();
      last_change_ms = g_ms;
      commutate_from_hall(last_hall, ccr, g_dir_forward);
    }
  }
}
