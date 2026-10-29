<div align="center">

  <img src="https://github.com/user-attachments/assets/a4dc6511-577b-4050-874c-d8b678992117" alt="EvilDuck Banner" width="100%"/>

  <br/>
  <br/>

  <!-- Badges -->
  <a href="https://github.com/cifertech/evilduck">
    <img src="https://img.shields.io/static/v1?label=cifertech&message=evilduck&color=orange&logo=github" alt="cifertech - EvilDuck"/>
  </a>
  &nbsp;
  <a href="https://github.com/cifertech/evilduck">
    <img src="https://img.shields.io/github/stars/cifertech/evilduck?style=social" alt="Stars"/>
  </a>
  &nbsp;
  <a href="https://github.com/cifertech/evilduck">
    <img src="https://img.shields.io/github/forks/cifertech/evilduck?style=social" alt="Forks"/>
  </a>
  &nbsp;
  <img src="https://img.shields.io/badge/ESP32--S3-WiFi%20%2B%20USB%20HID-orange?logo=espressif" alt="ESP32-S3"/>
  &nbsp;
  <img src="https://img.shields.io/badge/license-MIT-orange" alt="MIT License"/>

  <br/>
  <br/>

  <p>
  <a href="https://twitter.com/techcifer"><img src="https://img.shields.io/badge/Twitter-orange?logo=x&logoColor=black"/></a>
  <a href="https://www.instagram.com/cifertech/"><img src="https://img.shields.io/badge/Instagram-orange?logo=instagram&logoColor=black"/></a>
  <a href="https://www.youtube.com/@techcifer"><img src="https://img.shields.io/badge/YouTube-orange?logo=youtube&logoColor=black"/></a>
  <a href="https://cifertech.net/"><img src="https://img.shields.io/badge/Website-orange?logo=googlechrome&logoColor=black"/></a>
</p>

</div>

<br/>



## 📖 About the Project
 
EvilDuck is a custom USB rubber ducky — a device that plugs into any computer and acts as a keyboard, executing pre-written payloads automatically. It's inspired by [WiFiDuck](https://github.com/spacehuhntech/WiFiDuck) by Spacehuhn, which pairs an ESP8266 for WiFi with an ATmega32U4 for USB HID. EvilDuck takes that concept and builds on it with each hardware revision.
 
This repo covers **two generations** of the design:
 
| Version | Chip(s) | WiFi | SD Card | Web Interface |
|---|---|---|---|---|
| **EvilDuck SD** | Arduino Micro (ATmega32U4) | ❌ | ✅ | ❌ |
| **EvilDuck S3** | ESP32-S3 | ✅ | ✅ | ✅ |


<img width="1919" height="1080" alt="IMG_0537" src="https://github.com/user-attachments/assets/286fb130-bead-4b07-821e-4ae5140b8cf4" />

 
---
 
## 🦆 EvilDuck SD
 
The original version. Simple, standalone, and effective. Scripts are stored on an SD card and executed automatically when the device is powered. No WiFi, no web interface — just plug in, run.
 
### Features
 
- **Keystroke Injection** via Arduino Micro acting as a USB HID keyboard
- **SD Card Payload Storage** — scripts stored as `script.txt` on FAT16/32
- **Hot-Swap Detection** — detects SD card insertion while powered
- **LED Status Indicators**:
  - Blinks every 1s → no SD card detected
  - Blinks every 2s → no script file found
  - Blinks on each keystroke during execution
- **Rubber Ducky-style scripting** — supports `STRING`, `DELAY`, and special keypresses
### Hardware
 
| Component | Detail |
|---|---|
| Microcontroller | Arduino Micro (ATmega32U4) |
| Storage | MicroSD card (FAT16/32) |
| Status LED | Connected to Pin 9 |
| SD Card Module | SPI interface |
 
### Wiring
 
| SD Card Pin | Arduino Pin |
|---|---|
| CS | 4 |
| MOSI | 11 |
| MISO | 12 |
| SCK | 13 |
 
LED: Anode → Pin 9, Cathode → GND
 
### Software Requirements
 
- Arduino IDE 1.8+
- Libraries: `SD`, `SPI`, `Keyboard`
---
 
## 🦆 EvilDuck S3
 
The second generation. Built around the **ESP32-S3**, which handles both USB HID and WiFi natively on a single chip — no second microcontroller needed. The result is a more compact, more capable, fully self-contained device.
 
It adds a browser-based control panel over WiFi, SD card support for expanded storage, an addressable RGB LED, and a full DuckScript interpreter — all while staying the same physical size.
 
### What's New
 
- **Single-chip design** — ESP32-S3 replaces the ATmega32U4 + ESP8266 combo
- **WiFi access point** — browser-based control panel, no app required
- **Captive Portal DNS** — automatic captive redirection (`/generate_204`, `/hotspot-detect.html`) for instant phone setup
- **Multi-pin Addressable RGB LED** — WS2812 driving pins 1, 48, 38, 21 with startup rainbow animation and dynamic brightness control
- **SD card support** with expanded storage and USB mass storage mode
- **Over-the-air firmware updates** via the web interface
- **DuckScript interpreter** — executes scripts line by line as USB HID keystrokes with optimized comment skipping
- **Autorun** — plug in and a chosen script starts automatically
- **Stealth & Power Control** — adjusts timing, LED behavior, and switchable Wi-Fi TX power
- **AI Agent Skill Guide** — comprehensive development and troubleshooting runbook in [`SKILL.md`](./SKILL.md) and [`.agents/skills/esp32-embedded-development/SKILL.md`](./.agents/skills/esp32-embedded-development/SKILL.md)
### Hardware
 
| Component | Detail |
|---|---|
| Microcontroller | ESP32-S3 |
| WiFi | Built-in (802.11 b/g/n) |
| USB | Native USB HID (no second MCU) |
| Storage | Internal SPIFFS + MicroSD |
| RGB LED | WS2812 (2020 package) |
| Voltage Regulator | LM1117 (5V → 3.3V) |
| Connector | USB Type-A |
 
### Web Interface Features
 
Accessible over WiFi once the device is powered:
 
- Upload, edit, save, and delete scripts
- Trigger payloads remotely
- Live execution status (current script + line number)
- Execution log with warnings and errors
- Stop a running script mid-execution
- Send individual commands for quick testing
- Built-in script library (Windows, Linux, macOS, demos, advanced)
- Password-protected with optional hidden SSID
- Configurable network name, password, and channel
### Storage Modes
 
- **Internal flash** (SPIFFS) — persistent across reboots
- **SD card** — expanded storage, file transfer between internal and SD
- **USB mass storage mode** — drag and drop files directly
### Device Modes
 
| Mode | Description |
|---|---|
| Keyboard only | Acts purely as HID keyboard |
| Storage only | Acts as USB mass storage |
| Combined | Both keyboard and storage |
| Disabled | Neither |
 
---
 
## 🔌 Schematics
 
Full schematics for both versions are available in the `/Schematic` folder, and detailed build guides can be found on the [project website](https://cifertech.net/).
 
---
 
## 📄 License
 
Distributed under the MIT License. See `LICENSE` for details.
 
---
 
## 🤝 Contact
 
▶ Support me on Patreon: [patreon.com/cifertech](https://www.patreon.com/cifertech)
 
CiferTech — [@twitter](https://twitter.com/techcifer) — CiferTech@gmail.com
 
Project Link: [https://github.com/cifertech/EvilDuck](https://github.com/cifertech/EvilDuck)


