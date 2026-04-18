# IIOT-trained-by-CSIR-CEERI

# 🔬 CSIR-CEERI Embedded Systems Training — ESP32 Projects

> **Trainee Repository** | CSIR-Central Electronics Engineering Research Institute (CSIR-CEERI)  
> A collection of all ESP32/Arduino projects built and progressively developed during my training program.

---

## 👋 About This Repository

This repo documents my hands-on journey through embedded systems development at **CSIR-CEERI**, one of India's premier research institutions in electronics engineering. Starting from the very basics of GPIO control, and progressing all the way to a full **touchscreen-based Smart Attendance Panel** with Wi-Fi connectivity and Google Sheets integration — every file here represents a real milestone achieved during the training.

The projects are written in **Arduino C++** for the **ESP32 microcontroller** and cover a wide range of topics: timers, ADC, PWM, SPI, display drivers, keypad interfacing, web servers, and IoT connectivity.

---

## 🗂️ Project Index

| # | File | Category | Description |
|---|------|----------|-------------|
| 1 | `pb_led.ino` | GPIO Basics | Push button toggles LED with debounce |
| 2 | `blinking_with_timer.ino` | Timers & Interrupts | Hardware timer-based LED blinking (no `delay()`) |
| 3 | `ADC_led.ino` | ADC | Potentiometer controls LED states via voltage thresholds |
| 4 | `adc_and_blinking_with_timer.ino` | ADC + Timers | Dual hardware timers: one for LED blink, one for ADC sampling |
| 5 | `keypad.ino` | Peripherals | 4×4 matrix keypad using the Keypad library |
| 6 | `keypad_without_library.ino` | Peripherals | 4×4 keypad using raw column-scanning (no library) |
| 7 | `lcd_without_library.ino` | Display | 16×2 LCD driver written from scratch using 4-bit mode |
| 8 | `pass_led.ino` | Security | Keypad + LCD password system with lockout after 3 failed attempts |
| 9 | `pwm_with_motor.ino` | Motor Control | DC motor speed control using ESP32 LEDC PWM |
| 10 | `SPI_volt.ino` | SPI / ADC | High-precision voltage reading via ADS1120 SPI ADC |
| 11 | `motor_control_with_webhosting.ino` | IoT / Web Server | Motor controlled via a browser-based dashboard hosted on ESP32 |
| 12 | `TFT_LCD_display_celebration.ino` | TFT Display | Touchscreen calibration utility for XPT2046 + TFT_eSPI |
| 13 | `WIFI_preference_with_TFTlcd_display.ino` | IoT + TFT | Wi-Fi network selector with touchscreen keyboard & attendance logging |
| 14 | `smart_panel_fixed.ino` | Smart Panel v1 | First iteration of the Smart Attendance Panel |
| 15 | `smart_panel_fixed_2.ino` | Smart Panel v2 | Refined UI, improved state machine |
| 16 | `smart_panel_fixed_3.ino` | Smart Panel v3 | Added roll number input and student lookup |
| 17 | `smart_panel_fixed_4.ino` | Smart Panel v4 ✅ | Final version — full touchscreen attendance system with clock, Google Sheets sync |

---

## 📁 Detailed Project Descriptions

### 1. `pb_led.ino` — Push Button LED Toggle
**Concepts:** Digital I/O, pull-up resistors, debouncing  
A simple but essential starting point. A push button (GPIO 4, INPUT_PULLUP) toggles an LED (GPIO 2) on each press. Implements software debouncing with state-change detection rather than a naive `while pressed` loop.

---

### 2. `blinking_with_timer.ino` — Hardware Timer LED Blink
**Concepts:** ESP32 hardware timers, ISR (Interrupt Service Routine), IRAM_ATTR  
Blinks an LED at exactly 1-second intervals using ESP32's `hw_timer_t`. The entire `loop()` is empty — all logic happens inside the timer ISR. This is the correct, non-blocking approach to periodic tasks on embedded systems.

---

### 3. `ADC_led.ino` — ADC Voltage-Controlled LEDs
**Concepts:** 12-bit ADC, analog attenuation, voltage mapping  
Reads a potentiometer on GPIO 35 using the ESP32's 12-bit ADC (0–4095 range, 3.3V reference). Converts the raw value to actual voltage and controls two LEDs based on voltage thresholds (1.3V–1.8V → LED1 on; 1.8V–3.3V → LED2 on).

---

### 4. `adc_and_blinking_with_timer.ino` — Dual Timer: ADC + LED Blink
**Concepts:** Multiple hardware timers, volatile flags, ISR-safe patterns  
Uses **two independent hardware timers simultaneously**:
- **Timer 0** — toggles an LED every 1 second via ISR
- **Timer 1** — sets a `volatile bool` flag every 200ms, which the main `loop()` polls to trigger ADC reads safely

Demonstrates the correct pattern for sharing data between ISRs and the main loop.

---

### 5. `keypad.ino` — 4×4 Keypad with Library
**Concepts:** Matrix keypads, library usage  
Interfaces a 16-key matrix keypad using the standard Arduino `Keypad` library. Maps physical keys to characters (`1–9`, `A–D`, `*`, `#`) and prints the pressed key to Serial.

---

### 6. `keypad_without_library.ino` — 4×4 Keypad (Raw Column Scanning)
**Concepts:** GPIO matrix scanning, low-level hardware control  
The same 4×4 keypad — but implemented **entirely from scratch** without any library. Drives each column LOW one at a time and checks which row reads LOW, implementing full column-scan logic manually. A great exercise in understanding what libraries actually do under the hood.

---

### 7. `lcd_without_library.ino` — 16×2 LCD Driver from Scratch
**Concepts:** HD44780 LCD protocol, 4-bit communication, nibble transmission  
Drives a standard 16×2 LCD display using **pure GPIO bit-banging** in 4-bit mode — no `LiquidCrystal` library. Implements `lcdCommand()`, `lcdData()`, `lcdInit()`, `lcdClear()`, and `lcdPrint()` functions from scratch, manually toggling the Enable pin and sending nibbles in the correct HD44780 sequence.

---

### 8. `pass_led.ino` — Keypad + LCD Password Security System
**Concepts:** String comparison, state management, security lockout  
A complete password-entry system using:
- **Keypad** for input (`#` to confirm, `*` to clear)
- **LCD** for masked display (`*` for each digit)
- **LED1** (green) flashes on correct password → "Access Granted"
- **LED2** (red) flashes on wrong password → "Wrong Password"
- After **3 failed attempts**, the system locks for 10 seconds displaying "SYSTEM LOCKED"

---

### 9. `pwm_with_motor.ino` — DC Motor PWM Speed Control
**Concepts:** ESP32 LEDC peripheral, PWM, H-bridge (L298N)  
Controls a DC motor via an L298N H-bridge using the ESP32's LEDC PWM module. Sets up a 5kHz, 8-bit PWM channel on the ENA pin and drives the motor at full speed (PWM = 255) in the forward direction.

---

### 10. `SPI_volt.ino` — High-Precision Voltage via ADS1120 (SPI ADC)
**Concepts:** SPI protocol, external ADC, precision measurement  
Interfaces with the **ADS1120** — a 16-bit, SPI-based precision ADC — to measure differential voltage across pins AIN2–AIN3. Configures gain, data rate, conversion mode, and internal voltage reference via the `ADS1120` library, then reads and prints voltage values with 6 decimal places.

---

### 11. `motor_control_with_webhosting.ino` — IoT Motor Control Dashboard
**Concepts:** Wi-Fi, WebServer, PWM, REST-style HTTP routes  
The ESP32 hosts a **web dashboard** accessible from any browser on the same network. The page (styled with Tailwind CSS, dark theme) provides:
- A slider to set motor speed (0–100%)
- Buttons for Forward / Stop / Reverse
- Real-time fetch calls to `/forward`, `/reverse`, `/stop`, `/speed?value=N` routes on the ESP32

---

### 12. `TFT_LCD_display_celebration.ino` — Touchscreen Calibration Utility
**Concepts:** TFT_eSPI, XPT2046 resistive touch, calibration  
A calibration sketch for the XPT2046 resistive touchscreen controller paired with a TFT display. Draws crosshairs at two corners (top-left and bottom-right), collects 20 averaged touch readings per point, and outputs precise `map()` parameters to Serial Monitor for use in main application code.

---

### 13. `WIFI_preference_with_TFTlcd_display.ino` — Wi-Fi Manager + Attendance Logger
**Concepts:** TFT, touch UI, Wi-Fi scanning, HTTP POST, Google Sheets  
A multi-screen touchscreen application with a full on-screen keyboard:
- **HOME** screen with navigation
- **Wi-Fi List** — scans and lists available networks
- **Password Entry** — on-screen keyboard to enter Wi-Fi password
- **Connecting** screen with live status
- **Attendance** screen — logs entries to **Google Sheets** via a Google Apps Script webhook (HTTP GET)

---

### 14–17. `smart_panel_fixed.ino` → `smart_panel_fixed_4.ino` — Smart Attendance Panel (v1 → v4)
**Concepts:** Full-stack embedded IoT, state machines, NTP-like clock, Google Sheets API  

This is the **capstone project** of the training, developed across 4 iterative versions:

| Version | Key Addition |
|---------|-------------|
| v1 | Basic TFT UI and Wi-Fi connection flow |
| v2 | Refined state machine, improved button layout |
| v3 | Roll number input screen, student name lookup |
| v4 ✅ | Software clock (`HH:MM:SS`), student-specific attendance page, Google Sheets sync, full polish |

**Final version (v4) features:**
- 5-screen navigation: HOME → Wi-Fi List → Password → Roll Input → Student Panel
- Live clock display calculated from boot time
- Attendance marked by roll number, synced to Google Sheets via HTTP GET to a Google Apps Script URL
- Fully touch-driven with on-screen alphanumeric keyboard
- Clean dark-themed TFT UI with color-coded buttons

---

## 🛠️ Hardware Used

- **Microcontroller:** ESP32 (WROOM-32 / DevKit)
- **Display:** 2.8" TFT LCD (ILI9341) with XPT2046 resistive touch
- **ADC:** ADS1120 (16-bit SPI precision ADC)
- **Motor Driver:** L298N H-Bridge
- **Input:** 4×4 Matrix Keypad
- **Display:** 16×2 Character LCD (HD44780 compatible)
- **Other:** Potentiometer, LEDs, Push buttons

---

## 📚 Libraries Used

| Library | Purpose |
|---------|---------|
| `TFT_eSPI` | TFT display rendering |
| `XPT2046_Touchscreen` | Resistive touch input |
| `Keypad` | Matrix keypad (where used) |
| `LiquidCrystal` | LCD (where used) |
| `ADS1120` | SPI precision ADC |
| `WiFi.h` | ESP32 Wi-Fi stack |
| `WebServer.h` | HTTP server on ESP32 |
| `HTTPClient.h` | Outbound HTTP requests |

---

## 📈 Learning Progression

```
GPIO → Timers/ISR → ADC → Keypad → LCD → PWM/Motor
  → SPI ADC → Web Server → TFT Display → Touch UI → Full IoT System
```

---

## 🏛️ About CSIR-CEERI

The **Central Electronics Engineering Research Institute (CSIR-CEERI)**, Pilani, is one of India's leading R&D institutions under the Council of Scientific and Industrial Research (CSIR). It specializes in electronics, photonics, microelectronics, and embedded systems research.

---

## 📄 License

This repository is shared for educational and community reference purposes.  
Feel free to use, modify, and learn from any code here.

---

*Built with ☕ and a soldering iron at CSIR-CEERI.*
