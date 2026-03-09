# BLDC-6-step-UART

6-step Hall-sensored BLDC motor control firmware for **GD32F130C8T6**, controlled over **UART**.

This project uses:
- **GD32 SPL** with **PlatformIO**
- **6-step commutation** with Hall sensor feedback
- **UART command interface** for runtime motor control
- **ST-Link** for firmware upload

> **Project status:** experimental / development-stage. The original repository README currently states that the code is not fully working yet and is intended for development.

---

## Features

- 6-step BLDC commutation
- Hall sensor based sector detection
- UART command parser
- Direction control
- Brake mode
- Stall recovery with kick-start
- Hall-sector detection mode
- Runtime sector shift adjustment

---

## Hardware Target

### MCU
- **GD32F130C8T6**

### UART
- **USART1**
- **PA2** -> TX
- **PA3** -> RX
- **115200 8N1**

### PWM Outputs
- **PA8** -> TIMER0 CH0
- **PA9** -> TIMER0 CH1
- **PA10** -> TIMER0 CH2

### Low-side / LIN Outputs
- **PB13** -> LIN_U
- **PB14** -> LIN_V
- **PB15** -> LIN_W

### Hall Inputs
- **PB11** -> Hall A
- **PA1** -> Hall B
- **PC14** -> Hall C

> **Warning:** `PC14` shares functionality with the LSE oscillator pin. If your board uses LSE/RTC hardware, Hall C may not behave correctly.

---

## Repository Structure

```text
.
├── include/
│   ├── cmd.h
│   ├── hal_bldc.h
│   ├── pwm_lin.h
│   ├── timebase.h
│   └── uart.h
├── src/
│   ├── cmd.c
│   ├── hal_bldc.c
│   ├── main.c
│   ├── pwm_lin.c
│   ├── timebase.c
│   └── uart.c
└── platformio.ini
```

---

## How It Works

The firmware initializes UART, timebase, PWM, low-side outputs, and Hall inputs. After boot:

1. The board waits for UART commands.
2. A speed-related command (`DRV` or `DUTY`) starts the motor.
3. Hall sensors are read continuously.
4. Commutation is updated whenever Hall state changes.
5. If the rotor appears stalled, the firmware attempts a kick-start sequence.
6. Commands such as `STOP`, `BRAKE`, `DIR`, `SHIFT`, and `DETECT` can be sent at runtime.

---

# Loading Tutorial

## 1) Requirements

Install the following:

- **VS Code**
- **PlatformIO extension**
- **ST-Link driver / tools**
- A **USB-UART adapter** for sending commands and reading debug output

You also need:
- A **GD32F130C8T6** board
- A compatible **3-phase gate driver / BLDC power stage**
- A **Hall-sensored BLDC motor**
- External motor power supply

---

## 2) Clone the repository

```bash
git clone https://github.com/kaansonmez-stud/BLDC-6-step-UART.git
cd BLDC-6-step-UART
```

---

## 3) Open the project in VS Code

Open the folder in VS Code and let PlatformIO load the project.

The current project configuration targets:

```ini
[env:genericGD32F130C8]
platform = https://github.com/CommunityGD32Cores/platform-gd32.git
board = genericGD32F130C8
framework = spl
upload_protocol = stlink
debug_tool = stlink
```

---

## 4) Connect ST-Link

Connect your ST-Link to the GD32 board:

- **SWDIO**
- **SWCLK**
- **GND**
- **3.3V** (if required by your board/debugger setup)

Then power the board correctly.

---

## 5) Build the firmware

```bash
pio run
```

If the build succeeds, upload it with:

```bash
pio run -t upload
```

---

## 6) Open serial monitor

The firmware UART is configured for **115200 baud**.

Open monitor:

```bash
pio device monitor -b 115200
```

After reset, you should see something like:

```text
BOOT
Komutlar: DRV DUTY DIR STOP BRAKE
```

Depending on the code revision, extra debug lines may also appear.

---

# Wiring Tutorial

## ST-Link connection

| ST-Link | GD32F130C8T6 |
|---|---|
| SWDIO | SWDIO |
| SWCLK | SWCLK |
| GND | GND |
| 3.3V | 3.3V (optional / as needed) |

## UART adapter connection

| USB-UART | GD32 |
|---|---|
| TX | PA3 (MCU RX) |
| RX | PA2 (MCU TX) |
| GND | GND |

> TX/RX must be crossed.

## Motor stage connection overview

| Function | MCU Pin |
|---|---|
| High-side PWM U | PA8 |
| High-side PWM V | PA9 |
| High-side PWM W | PA10 |
| Low-side U | PB13 |
| Low-side V | PB14 |
| Low-side W | PB15 |
| Hall A | PB11 |
| Hall B | PA1 |
| Hall C | PC14 |

> Exact power-stage wiring depends on your MOSFET/gate-driver hardware.

---

# Usage Tutorial

## Command format

Commands are sent as plain text over UART.

Examples:

```text
DRV 40
DUTY 15
DIR 1
STOP
BRAKE
SHIFT 1
DETECT
```

Commands are line-based. Send `Enter` / newline after each command.

---

## Available Commands

### `DRV <0..255>`
Set drive level using an 8-bit value.

Example:
```text
DRV 60
```

Use this when you want direct control over the internal CCR scaling.

---

### `DUTY <0..100>`
Set PWM duty in percent.

Example:
```text
DUTY 20
```

The code limits duty to a safe upper bound internally.

---

### `DIR <1|0>`
Set motor direction.

- `DIR 1` -> forward
- `DIR 0` -> reverse

Example:
```text
DIR 1
```

---

### `STOP`
Disable motor drive and turn outputs off.

Example:
```text
STOP
```

---

### `BRAKE`
Apply braking by enabling all low-side switches.

Example:
```text
BRAKE
```

> Use brake carefully. This can create strong braking torque and current spikes depending on your hardware.

---

### `SHIFT <-5..5>`
Adjust sector alignment offset.

Example:
```text
SHIFT 1
```

Useful if Hall-to-sector mapping needs runtime correction.

---

### `DETECT`
Runs an automatic Hall-sector detection routine.

Example:
```text
DETECT
```

Typical output:

```text
DETECT sonuc:
  S0 -> hall=...
  S1 -> hall=...
  S2 -> hall=...
  S3 -> hall=...
  S4 -> hall=...
  S5 -> hall=...
DETECT bitti
```

---

## Recommended first test sequence

Start carefully with low values:

```text
DIR 1
DUTY 5
DUTY 10
DUTY 15
STOP
```

If the motor does not start correctly:

```text
DETECT
SHIFT 1
DUTY 5
```

Try different `SHIFT` values from `-5` to `+5` if Hall alignment is incorrect.

---

## Debug Output

The firmware periodically prints diagnostic information, such as:

```text
DBG: h=...
DBG DRV: hall=...
DBG DUTY: hall=...
DBG STALL: kick h=...
```

These messages help you verify:
- Hall sensor state
- Whether Hall values are valid
- Active CCR / duty
- Whether kick-start was triggered

---

## Safety Notes

- Start with **very low** `DUTY` or `DRV` values.
- Keep the motor shaft unloaded during first tests.
- Use a current-limited bench supply if possible.
- Make sure **MCU ground**, **UART ground**, and **power stage ground** are common.
- Never assume the direction is correct on the first spin.
- Be careful with `BRAKE` because it can stress the inverter and power supply.

---

## Known Limitations

- The repository is still in development.
- Exact driver-stage electrical compatibility depends on your hardware.
- Hall wiring/order may require `SHIFT` tuning or phase remapping.
- `PC14` may be problematic on boards using LSE hardware.

---

## Useful PlatformIO Commands

Build:
```bash
pio run
```

Upload:
```bash
pio run -t upload
```

Clean:
```bash
pio run -t clean
```

Serial monitor:
```bash
pio device monitor -b 115200
```

---

## License

Add your preferred license here.

---

## Author

Repository owner: **kaansonmez-stud**

You can extend this README later with:
- motor/driver schematic
- oscilloscope screenshots
- tested hardware list
- Hall waveform examples
- UART demo video
