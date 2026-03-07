Tamam. Aşağıdaki **tek bir Markdown kod bloğu**.
**Baştan sona tek seferde kopyalayabilirsin.**

```markdown
# BLDC 6-Step Motor Control with UART

A lightweight firmware implementation of **sensor-based BLDC motor control using 6-step commutation** with a **UART command interface**.

This project demonstrates how to drive a **3-phase BLDC motor** using Hall sensor feedback and control it interactively from a **serial terminal**.

The firmware is intended for **embedded experimentation, motor driver testing, and educational purposes**.

---

# Project Overview

The motor is controlled using **trapezoidal (6-step) commutation**.

Rotor position is determined using **Hall sensors**, and the firmware switches the motor phases accordingly.

Motor commands are sent through **UART**, allowing direct control from a PC terminal.

Typical use cases:

- BLDC driver bring-up  
- Motor control experiments  
- Embedded firmware learning  
- Testing custom MOSFET bridge drivers  

---

# System Architecture

```

PC / Serial Terminal
│
│ UART
│
┌──────────────────┐
│  Microcontroller │
│                  │
│  UART Interface  │
│  Motor Control   │
│  PWM Generator   │
└─────────┬────────┘
│
│ PWM + GPIO
│
┌─────────────────────┐
│ 3-Phase MOSFET Gate │
│      Driver         │
└─────────┬───────────┘
│
BLDC Motor
│
Hall Sensors
│
MCU Inputs

````

---

# Features

- 6-step trapezoidal BLDC commutation  
- Hall sensor rotor position detection  
- UART based motor control  
- PWM speed control  
- Dynamic braking support  
- Minimal and easy-to-understand firmware structure  

---

# Hardware Requirements

Example hardware configuration:

| Component | Description |
|-----------|-------------|
| Microcontroller | GD32F130 / STM32 or compatible MCU |
| Motor | 3-phase BLDC motor with Hall sensors |
| Driver | 3-phase MOSFET bridge |
| UART Adapter | FTDI / CP2102 / CH340 |
| Power Supply | Suitable DC supply for motor |

---

# BLDC Commutation Principle

The motor uses **6-step trapezoidal commutation**.

At each step:

- one phase is driven **high (PWM)**  
- one phase is driven **low**  
- one phase is **floating**

Example commutation table:

| Hall | Phase A | Phase B | Phase C |
|------|---------|---------|---------|
| 001 | + | − | Z |
| 101 | + | Z | − |
| 100 | Z | + | − |
| 110 | − | + | Z |
| 010 | − | Z | + |
| 011 | Z | − | + |

Legend:

+ : High-side PWM  
- : Low-side active  
Z : Floating phase  

---

# Braking Method

The firmware implements **dynamic braking (short-circuit braking)**.

During braking the following functions are executed:

```c
pwm_all_off();
lin_all_on();
````

Effect:

* PWM outputs disabled
* All low-side MOSFETs enabled

This electrically connects all motor phases to **ground**, effectively creating a **short circuit across the windings**.

The motor’s **back-EMF generates current inside the windings**, dissipating energy as heat and rapidly slowing the rotor.

---

# UART Command Interface

Motor control is performed using simple UART commands.

Default configuration:

```
Baudrate: 115200
Data bits: 8
Parity: None
Stop bits: 1
```

Command list:

| Command | Description                  |
| ------- | ---------------------------- |
| f       | Start motor forward          |
| b       | Start motor backward         |
| s       | Stop motor (dynamic braking) |

Example usage:

```
f
```

Motor starts rotating forward.

```
b
```

Motor rotates backward.

```
s
```

Motor stops using dynamic braking.

---

# Software Structure

Example project structure:

```
src
│
├── main.c
├── motor_control.c
├── motor_control.h
├── uart.c
├── uart.h
└── driver_interface.c
```

Module overview:

| Module           | Description                           |
| ---------------- | ------------------------------------- |
| main             | System initialization and main loop   |
| uart             | UART communication and command parser |
| motor_control    | BLDC commutation logic                |
| driver_interface | PWM and MOSFET control                |

---

# Firmware Flow

```
System Initialization
        │
        ▼
UART Initialization
        │
        ▼
PWM Initialization
        │
        ▼
Main Loop
        │
        ▼
Receive UART Command
        │
        ▼
Interpret Command
        │
        ▼
Update Motor State
```

---

# Safety Considerations

Dynamic braking can generate **high current spikes**, especially when stopping the motor from high speed.

Recommendations:

* Ensure MOSFETs are rated for braking current
* Provide sufficient cooling
* Test braking at low speed first
* Monitor motor temperature during testing

---

# Possible Improvements

Potential future improvements:

* Closed-loop speed control (PID)
* Current limiting
* Soft start ramp
* Stall detection
* Regenerative braking
* Field Oriented Control (FOC)

---

# License

This project is intended for **educational and experimental purposes**.

```
```
