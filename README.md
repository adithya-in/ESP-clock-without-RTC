# ESP8266 Digital Clock with MAX7219 Display & Web Alarm
![WhatsApp Image 2025-09-06 at 12 23 20 AM (1)](https://github.com/user-attachments/assets/f4563f43-1058-4aab-8a9f-432545044cc8)

## 📌 Project Overview

This project is a **Wi-Fi enabled digital clock** built using an **ESP8266** and a **4-in-1 MAX7219 LED matrix display**.
It shows the current **time** and **scrolling date**, and also includes a **buzzer alarm** feature configurable through a **web interface**.

Designed as a simple IoT project for beginners and hobbyists, it combines **hardware, firmware, and web control** in one neat build.

---

## ✨ Features

* ⏰ Real-time clock display
* 📅 Date scrolling across MAX7219 matrix
* 🔊 Buzzer alarm with on/off toggle
* 🌐 Web-based alarm configuration
* 💾 NTP (Network Time Protocol) sync for accurate timekeeping
* ⚡ Compact design with ESP8266 & LED display

---

## 🛠️ Hardware Required

* ESP8266 board (NodeMCU / Wemos D1 Mini)
* 4-in-1 MAX7219 LED Matrix display
* Active Buzzer
* Jumper wires & breadboard / custom PCB
* USB cable & power supply

---

## 🔌 Circuit Connections

| MAX7219 | ESP8266 (NodeMCU) |
| ------- | ----------------- |
| VCC     | 3.3V              |
| GND     | GND               |
| DIN     | D7 (GPIO13)       |
| CS      | D8 (GPIO15)       |
| CLK     | D5 (GPIO14)       |

**Buzzer**: Connect to D2 (GPIO4) with GND.

---

## 💻 Software Setup

1. Install [Arduino IDE](https://www.arduino.cc/en/software)

2. Add **ESP8266 board support** (via Boards Manager)

3. Install required libraries:

   * `ESP8266WiFi.h`
   * `ESP8266WebServer.h`
   * `NTPClient.h`
   * `WiFiUdp.h`
   * `MD_Parola` and `MD_MAX72XX` for MAX7219

4. Flash the code to your ESP8266.

---

## 🌐 Web Interface

* Connect ESP8266 to Wi-Fi.
* Open its IP in a browser.
* Set alarm time & enable/disable buzzer.

![WhatsApp Image 2025-09-06 at 12 26 07 AM](https://github.com/user-attachments/assets/bb7c5c3f-6468-4e14-b33b-a758f0e15dcd)
