# Lab 4 — Motor Control using RTOS
### Embedded Systems Development · 04-633 A · Carnegie Mellon University Africa

> **Due:** April 10, 2026 &nbsp;|&nbsp; **Demo:** April 13, 2026  
> **Team:** Abba Uba Said (`asaiduba`) · Jeremiah Dosu (`jdosu`)  
> **Board:** STM32 Nucleo-F103RB &nbsp;|&nbsp; **RTOS:** FreeRTOS (CMSIS-V1) &nbsp;|&nbsp; **IDE:** STM32CubeIDE

---

## Table of Contents
1. [Project Overview](#project-overview)
2. [System Architecture](#system-architecture)
3. [Hardware Requirements & Wiring](#hardware-requirements--wiring)
4. [Software Requirements](#software-requirements)
5. [Project Structure](#project-structure)
6. [FreeRTOS Tasks](#freertos-tasks)
7. [PWM & Timer Configuration](#pwm--timer-configuration)
8. [How to Build & Flash](#how-to-build--flash)
9. [How to Use the System](#how-to-use-the-system)
10. [Debugging Notes & Known Issues](#debugging-notes--known-issues)
11. [Submission](#submission)

---

## Project Overview

This project implements a **real-time servo motor controller** on the STM32 Nucleo-F103RB
using FreeRTOS for concurrent task management and hardware PWM (TIM2) for precise
SG90 servo control.

**The user experience:**
```
┌────────────────┐
│ Enter: 90_     │  ← Digits you type appear live with a blinking cursor
│ Set:   90 deg  │  ← Last confirmed servo angle
└────────────────┘
```
- Type an angle (0–180) on the **4×4 keypad**
- Press `*` to backspace, `#` to confirm
- The **SG90 servo** moves to that position instantly
- The **LCD** updates in real time as you type
- The **LD2 LED** on the Nucleo blinks every 3 s as an RTOS heartbeat

---

## System Architecture

```
┌─────────────┐    mutex     ┌──────────────┐    PWM     ┌──────────┐
│ KeypadTask  │─────────────►│  ServoTask   │───────────►│  SG90    │
│  (Normal)   │              │(AboveNormal) │  TIM2 CH1  │  Servo   │
└─────────────┘              └──────────────┘            └──────────┘
       │                            │
       │ mutex                      │ mutex
       ▼                            ▼
┌─────────────┐         targetAngle (uint8_t)
│  LCDTask    │         inputDisplay (char[5])
│  (Normal)   │──────► 16×2 I²C LCD (PCF8574)
└─────────────┘
       
┌──────────────┐
│ defaultTask  │──────► PA5 LED blink every 3 s (RTOS heartbeat)
│    (Low)     │
└──────────────┘
```

**Shared state** — both variables protected by `angleMutex`:

| Variable | Type | Written by | Read by |
|----------|------|-----------|---------|
| `targetAngle` | `volatile uint8_t` | KeypadTask | ServoTask, LCDTask |
| `inputDisplay[5]` | `volatile char[]` | KeypadTask | LCDTask |

---

## Hardware Requirements & Wiring

### Components
| Component | Specification |
|-----------|--------------|
| MCU Board | STM32 Nucleo-F103RB (STM32F103RBTx) |
| Servo | SG90 micro servo motor |
| Keypad | 4×4 matrix membrane keypad |
| LCD | 16×2 HD44780 with PCF8574 I²C backpack |
| Power | USB (3.3 V for MCU logic) + 5 V rail for servo |

### Pin Assignment

| Pin | Function | Connect To |
|-----|----------|-----------|
| **PA0** | TIM2_CH1 PWM | Servo signal wire (orange/yellow) |
| **PA5** | GPIO Output | Nucleo LD2 LED (on-board, no wiring needed) |
| **PB0** | Row 1 (OUTPUT) | Keypad Row 1 |
| **PB1** | Row 2 (OUTPUT) | Keypad Row 2 |
| **PB2** | Row 3 (OUTPUT) | Keypad Row 3 |
| **PB3** | Row 4 (OUTPUT) | Keypad Row 4 |
| **PB4** | Col 1 (INPUT Pull-Up) | Keypad Col 1 |
| **PB5** | Col 2 (INPUT Pull-Up) | Keypad Col 2 |
| **PB6** | Col 3 (INPUT Pull-Up) | Keypad Col 3 |
| **PB7** | Col 4 (INPUT Pull-Up) | Keypad Col 4 |
| **PB8** | I2C1_SCL (remapped) | LCD SCL |
| **PB9** | I2C1_SDA (remapped) | LCD SDA |
| **5V** | Power | Servo VCC (red wire) |
| **GND** | Ground | Servo GND + LCD GND |

### Keypad Layout
```
Physical key → function:
  [ 1 ][ 2 ][ 3 ][ A ]   Row 1 — PB0
  [ 4 ][ 5 ][ 6 ][ B ]   Row 2 — PB1
  [ 7 ][ 8 ][ 9 ][ C ]   Row 3 — PB2
  [ * ][ 0 ][ # ][ D ]   Row 4 — PB3
            ↑   ↑   ↑   ↑
           PB4 PB5 PB6 PB7   (Columns — input pull-up)
```

| Key | Action |
|-----|--------|
| `0`–`9` | Type digit into angle buffer |
| `*` | Backspace — delete last digit |
| `#` | Confirm — send angle to servo |
| `A` `B` `C` `D` | Ignored |

> ⚠️ **PB3 (Row 4)** is a JTAG pin. It is freed by `__HAL_AFIO_REMAP_SWJ_NOJTAG()` in `HAL_MspInit()`. Do **not** change this to `SWJ_DISABLE` — that will lock out the ST-Link debugger.

---

## Software Requirements

- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) ≥ 1.14
- STM32Cube FW_F1 V1.8.7 (auto-managed by CubeMX)
- FreeRTOS kernel V10.3.1 (included via STM32 middleware)
- No external libraries required — all drivers (`keypad.c`, `lcd_i2c.c`, `servo.c`) are custom implementations included in this repo

---

## Project Structure

```
FREEROS/
├── Core/
│   ├── Inc/
│   │   ├── main.h              # HAL includes, function prototypes
│   │   ├── FreeRTOSConfig.h    # RTOS configuration (1kHz tick, 8KB heap)
│   │   ├── keypad.h            # Keypad pin definitions & API
│   │   ├── lcd_i2c.h           # LCD I²C address & API
│   │   └── servo.h             # Servo pulse constants & API
│   └── Src/
│       ├── main.c              # Peripheral init, task creation, shared vars
│       ├── freertos.c          # RTOS idle memory (auto-generated)
│       ├── keypad.c            # 4×4 matrix scan driver (debounce, release-wait)
│       ├── lcd_i2c.c           # HD44780 4-bit driver via PCF8574 I²C
│       ├── servo.c             # SG90 driver — HAL_TIM_PWM_Start + angle→pulse
│       ├── stm32f1xx_hal_msp.c # GPIO/I²C/TIM peripheral clock & NVIC config
│       └── stm32f1xx_it.c      # Interrupt handlers (TIM4 → HAL_IncTick)
├── Drivers/                    # STM32 HAL (auto-generated, do not edit)
├── Middlewares/                # FreeRTOS kernel (auto-generated, do not edit)
├── FREEROS.ioc                 # STM32CubeMX project config
├── STM32F103RBTX_FLASH.ld      # Linker script
└── report/
    └── lab4_report.tex         # LaTeX individual report
```

---

## FreeRTOS Tasks

### Task 1 — `KeypadTask` (Priority: Normal, Stack: 256 words)
- Calls `Keypad_Init()` then loops calling `Keypad_GetKey()`
- Buffers up to 3 digits (`"0"` – `"180"`)
- `*` → backspace, `#` → parse with `atoi()`, clamp to [0, 180], write to `targetAngle` and clear `inputDisplay` under mutex
- Writes typed digits to `inputDisplay` after every keypress so the LCD can show them live

### Task 2 — `ServoTask` (Priority: AboveNormal, Stack: 256 words)
- Calls `Servo_Init(&htim2)` → starts `HAL_TIM_PWM_Start()` on TIM2 CH1
- Reads `targetAngle` under mutex every 20 ms
- Calls `Servo_SetAngle(angle)` which sets `__HAL_TIM_SET_COMPARE()` with the interpolated pulse width

### Task 3 — `LCDTask` (Priority: Normal, Stack: 256 words)
- Calls `LCD_Init(&hi2c1)` (HD44780 4-bit init sequence via PCF8574)
- Refreshes every 150 ms:
  - **Row 0:** `"Enter: XXX_    "` — live typing with blinking `_` cursor (toggles every ~450 ms)
  - **Row 1:** `"Set:  XXX deg  "` — confirmed servo angle

### Task 4 — `defaultTask` (Priority: Low, Stack: 128 words)
- Toggles PA5 (Nucleo LD2) every 3 000 ms
- Serves as a visual RTOS heartbeat — if the LED stops blinking, the scheduler has hung

---

## PWM & Timer Configuration

### System Clock: 72 MHz
```
HSE (8 MHz) → PLL ×9 → SYSCLK = 72 MHz
APB1 CLK divider = /2 → APB1 = 36 MHz
APB1 Timer clock = 72 MHz  (×2 when APB divider ≠ 1)
```

### TIM2 — Servo PWM (PA0, Channel 1)
```
Prescaler  = 72 - 1   → Timer clock = 72 000 000 / 72 = 1 MHz (1 µs/tick)
Period     = 20000 - 1 → PWM period  = 20 000 µs = 20 ms = 50 Hz ✅

Pulse width mapping (SG90):
  0°   → 500  ticks = 500  µs  (0.5 ms)
  90°  → 1500 ticks = 1500 µs  (1.5 ms)
  180° → 2500 ticks = 2500 µs  (2.5 ms)

Formula: pulse = 500 + (angle × 2000) / 180
```

### TIM4 — HAL Timebase (1 ms interrupt)
Used by HAL exclusively; separated from FreeRTOS SysTick to avoid tick conflicts.

---

## How to Build & Flash

```bash
# 1. Clone the repository
git clone <your-github-classroom-repo-url>

# 2. Open in STM32CubeIDE
File → Open Projects from File System → select the FREEROS/ folder

# 3. Build
Project → Build All   (Ctrl + B)
# Expected: 0 errors, 0 warnings

# 4. Flash
Run → Debug As → STM32 Cortex-M C/C++ Application
# Or: Run → Run As → STM32 Cortex-M C/C++ Application (no debug)
```

> Make sure the Nucleo board is connected via USB and the ST-Link driver is installed.

---

## How to Use the System

| Step | Action | LCD display |
|------|--------|------------|
| 1 | Power on | `Enter: _` / `Set:  90 deg` |
| 2 | Press `9`, `0` | `Enter: 90_` / `Set:  90 deg` |
| 3 | Press `#` | `Enter: _` / `Set:  90 deg` → servo at 90° |
| 4 | Press `1`, `8`, `0` | `Enter: 180_` / `Set:  90 deg` |
| 5 | Press `#` | `Enter: _` / `Set: 180 deg` → servo at 180° |
| 6 | Press `*` | Deletes last typed digit |

---

## Debugging Notes & Known Issues

### 1. Servo vibrating instead of moving
**Cause:** TIM2 prescaler/period wrong → PWM frequency too high (5 kHz instead of 50 Hz).  
**Fix:** `Prescaler = 72 - 1`, `Period = 20000 - 1` in `MX_TIM2_Init()`.

### 2. LCD shows nothing
**Cause:** I²C1 is remapped to PB8/PB9 in CubeMX but wires connected to PB6/PB7.  
**Fix:** Move SDA wire → **PB9**, SCL wire → **PB8** on the Nucleo CN10 header.

### 3. Board not detected by ST-Link after flash
**Cause:** `__HAL_AFIO_REMAP_SWJ_DISABLE()` disables SWD, locking out ST-Link.  
**Recovery:** Hold BOOT0 HIGH while powering on to enter bootloader mode, then reflash.  
**Fix:** Change to `__HAL_AFIO_REMAP_SWJ_NOJTAG()` in `stm32f1xx_hal_msp.c`.

### 4. Keypad bottom row (`* 0 # D`) unresponsive
**Cause:** PB3 (Row 4) = JTAG TDI — not released if SWJ remap order is wrong.  
**Fix:** Call `SWJ_NOJTAG` inside `HAL_MspInit()` (before `MX_GPIO_Init()` runs).

### 5. LCD I²C address
Default address is `0x27`. If LCD is blank after correct wiring, try `0x3F` in `lcd_i2c.h`:
```c
#define LCD_I2C_ADDR  (0x3F << 1)  // some modules ship with A0-A2 = HIGH
```

---

## Submission

This project follows the **Lab 4 Submission Guidelines** from the 04-633 A course handout.

---

### 8.1 Complete Source Code — GitHub Submission

- [x] All relevant project files committed, including driver files (`keypad.c`, `lcd_i2c.c`, `servo.c`) and `main.c`
- [x] `README.md` included with project description, wiring guide, and implementation notes
- [ ] **Create a GitHub issue titled `Lab4-Submission`** containing:
  - **Name:** Abba Uba Said &nbsp;|&nbsp; **Partner:** Jeremiah Dosu
  - **Andrew IDs:** `asaiduba@andrew.cmu.edu` · `jdosu@andrew.cmu.edu`
  - **Commit hash** used for grading
  - **Link** to individual Canvas report

> 📋 **Copy this into the GitHub issue body:**
> ```
> Title: Lab4-Submission
>
> Name:         Abba Uba Said
> Partner:      Jeremiah Dosu
> Andrew IDs:   asaiduba@andrew.cmu.edu, jdosu@andrew.cmu.edu
> Commit Hash:  <paste your final commit hash here>
> Report Link:  <paste your Canvas submission URL here>
> ```

---

### 8.2 Project Report — Individual Canvas Submission

Each student submits a **separate** individual report (max 3 pages) on Canvas, covering:

- Overview of the implementation
- How FreeRTOS and timer interrupts were used
- Challenges Faced, Solutions Implemented, and Key Lessons Learned

> The report source is at `report/lab4_report.tex`.  
> Compile it on [Overleaf](https://www.overleaf.com) (upload `lab4_report.tex` + `logo.png`), then download the PDF and submit to Canvas.

---

### 8.3 Demonstration — In-Lab Presentation (April 13, 2026)

Present the working project to the TA, demonstrating:

1. The complete working system running live on the Nucleo board
2. How FreeRTOS tasks were implemented and interact with each other
3. Proper PWM servo control using hardware timer interrupts (show servo responding to keypad input)

---

*Carnegie Mellon University Africa · Embedded Systems Development 04-633 A · 2026*
