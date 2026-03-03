#pragma once

#include <stdint.h>

/*
 * Komut sistemi: UART'tan satır okur, parse eder, callback ile bildirir.
 *
 * Komutlar:
 *   DRV <0..255>   -> drv değeri
 *   DUTY <0..100>  -> duty yüzdesi
 *   DIR <1|0>      -> yön
 *   STOP           -> durdur
 *   BRAKE          -> frenle
 */

/* Komut tipleri */
typedef enum {
  CMD_NONE = 0,
  CMD_DRV,
  CMD_DUTY,
  CMD_DIR,
  CMD_STOP,
  CMD_BRAKE,
  CMD_SHIFT,
  CMD_DETECT
} cmd_type_t;

/* Parse edilmiş komut */
typedef struct {
  cmd_type_t type;
  int16_t value; /* DRV: 0..255, DUTY: 0..100, DIR: 0/1 */
} cmd_result_t;

/* Komut satır buffer yönetimi */
typedef struct {
  char line[64];
  uint8_t li;
  uint32_t last_rx_ms;
} cmd_state_t;

void cmd_init(cmd_state_t *s);
cmd_result_t cmd_poll(cmd_state_t *s, uint32_t now_ms);
