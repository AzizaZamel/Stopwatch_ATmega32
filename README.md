# Stopwatch with Dual Mode

## Overview
This project implements a stopwatch with two modes: increment and countdown, using an ATmega32 microcontroller. It features a six-digit seven-segment display and control buttons for resetting, pausing, resuming, adjusting time, and switching between modes.

## Features
- **Increment Mode**: Counts up from zero.
- **Countdown Mode**: User sets a time and the stopwatch counts down.
- **Alarm**: A buzzer sounds when the countdown reaches zero.
- **Multiplexed Display**: Six seven-segment displays controlled via multiplexing and 7447 decoder.
- **Pause/Resume**: Allows pausing and resuming the stopwatch.
- **Reset**: Resets the stopwatch to zero.
- **Time Adjustment**: Buttons for adjusting hours, minutes, and seconds in countdown mode.
- **Mode Toggle**: Switches between increment and countdown modes.

## Hardware Components
- **Microcontroller**: ATmega32
- **Display**: Six multiplexed seven-segment displays (common anode) driven by a 7447 BCD to seven-segment decoder
- **Input**: 
  - 9 push buttons for:
    - Reset
    - Pause
    - Resume
    - Mode toggle
    - Adjust hours (increment/decrement)
    - Adjust minutes (increment/decrement)
    - Adjust seconds (increment/decrement)
- **Buzzer**: Sounds when the countdown reaches zero.
- **LED Indicators**: 
  - Red LED (PD4) for increment mode
  - Yellow LED (PD5) for countdown mode
