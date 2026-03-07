BLDC 6-Step Motor Control with UART

A lightweight firmware implementation of sensor-based BLDC motor control using 6-step commutation with a UART command interface.

This project demonstrates how to drive a 3-phase BLDC motor using Hall sensor feedback and control it interactively from a serial terminal.

The firmware is designed for embedded experimentation and motor driver development.

Project Overview

The motor is controlled using trapezoidal (6-step) commutation.
Rotor position is determined by Hall sensors, and phase switching is performed according to the detected Hall state.

Motor control commands are sent via UART, allowing simple testing and debugging from a PC.

Typical use cases:

BLDC driver bring-up

Motor control experiments

Embedded firmware learning

Testing custom MOSFET bridge drivers

System Architecture
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
