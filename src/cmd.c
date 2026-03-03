#include "cmd.h"
#include "uart.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* Tam kelime eşleşme (case-insensitive) */
static uint8_t match_cmd(const char *s, const char *cmd) {
  while (*cmd && *s) {
    if (tolower((unsigned char)*s) != tolower((unsigned char)*cmd))
      return 0;
    s++;
    cmd++;
  }
  return (*cmd == 0) && (*s == 0 || isspace((unsigned char)*s));
}

/* Satırı parse et -> cmd_result_t döndür */
static cmd_result_t parse_line(char *line) {
  cmd_result_t r = {CMD_NONE, 0};

  while (*line && isspace((unsigned char)*line))
    line++;
  if (*line == 0)
    return r;

  /* DRV <0..255> */
  if (match_cmd(line, "DRV")) {
    char *p = line + 3;
    while (*p && isspace((unsigned char)*p))
      p++;
    if (*p == 0 || (!isdigit((unsigned char)*p) && *p != '-')) {
      uart_print("ERR: DRV <0..255> bekleniyor\n");
      return r;
    }
    long v = strtol(p, NULL, 10);
    if (v < 0)
      v = 0;
    if (v > 255)
      v = 255;
    r.type = CMD_DRV;
    r.value = (int16_t)v;
    uart_print("OK DRV=");
    uart_print_int(r.value);
    uart_print("\n");
    return r;
  }

  /* DUTY <0..100> */
  if (match_cmd(line, "DUTY")) {
    char *p = line + 4;
    while (*p && isspace((unsigned char)*p))
      p++;
    if (*p == 0 || (!isdigit((unsigned char)*p) && *p != '-')) {
      uart_print("ERR: DUTY <0..100> bekleniyor\n");
      return r;
    }
    long v = strtol(p, NULL, 10);
    if (v < 0)
      v = 0;
    if (v > 100)
      v = 100;
    r.type = CMD_DUTY;
    r.value = (int16_t)v;
    uart_print("OK DUTY=");
    uart_print_int(r.value);
    uart_print("%\n");
    return r;
  }

  /* DIR <1|0> */
  if (match_cmd(line, "DIR")) {
    char *p = line + 3;
    while (*p && isspace((unsigned char)*p))
      p++;
    if (*p == '0') {
      r.type = CMD_DIR;
      r.value = 0;
      uart_print("OK DIR=0 (GERI)\n");
    } else if (*p == '1') {
      r.type = CMD_DIR;
      r.value = 1;
      uart_print("OK DIR=1 (ILERI)\n");
    } else {
      uart_print("ERR: DIR 0 veya 1 bekleniyor\n");
    }
    return r;
  }

  /* STOP */
  if (match_cmd(line, "STOP")) {
    r.type = CMD_STOP;
    uart_print("OK STOP\n");
    return r;
  }

  /* BRAKE */
  if (match_cmd(line, "BRAKE")) {
    r.type = CMD_BRAKE;
    uart_print("OK BRAKE\n");
    return r;
  }

  /* SHIFT <-5..5> */
  if (match_cmd(line, "SHIFT")) {
    char *p = line + 5;
    while (*p && isspace((unsigned char)*p))
      p++;
    if (*p == 0 || (!isdigit((unsigned char)*p) && *p != '-')) {
      uart_print("ERR: SHIFT <-5..5> bekleniyor\n");
      return r;
    }
    long v = strtol(p, NULL, 10);
    if (v < -5)
      v = -5;
    if (v > 5)
      v = 5;
    r.type = CMD_SHIFT;
    r.value = (int16_t)v;
    uart_print("OK SHIFT=");
    uart_print_int(r.value);
    uart_print("\n");
    return r;
  }

  /* DETECT */
  if (match_cmd(line, "DETECT")) {
    r.type = CMD_DETECT;
    uart_print("OK DETECT basladi...\n");
    return r;
  }

  uart_print("ERR: Bilinmeyen komut -> ");
  uart_print(line);
  uart_print("\nKomutlar: DRV DUTY DIR STOP BRAKE SHIFT DETECT\n");
  return r;
}

void cmd_init(cmd_state_t *s) {
  s->li = 0;
  s->last_rx_ms = 0;
  memset(s->line, 0, sizeof(s->line));
}

cmd_result_t cmd_poll(cmd_state_t *s, uint32_t now_ms) {
  cmd_result_t r = {CMD_NONE, 0};

  while (uart_rx_ready()) {
    char c = (char)uart_getc();
    s->last_rx_ms = now_ms;

    if (c == '\r' || c == '\n') {
      if (s->li > 0) {
        s->line[s->li] = 0;
        r = parse_line(s->line);
        s->li = 0;
        return r;
      }
      continue;
    }

    if (s->li < (sizeof(s->line) - 1U)) {
      s->line[s->li++] = c;
    } else {
      s->li = 0;
      uart_print("ERR: Satir cok uzun\n");
    }
  }

  /* Timeout: 300ms yeni karakter gelmezse komutu isle */
  if (s->li > 0 && (now_ms - s->last_rx_ms) >= 300U) {
    s->line[s->li] = 0;
    r = parse_line(s->line);
    s->li = 0;
  }

  return r;
}
