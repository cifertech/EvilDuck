---
name: esp32-embedded-development
description: Comprehensive operational runbook and engineering guidelines for building, debugging, flashing, and stabilizing ESP32 and ESP32-S3 firmware projects (including EvilDuck). Covers physical hardware recovery (buttons, boot loops, autorun lockouts, brownouts), dual USB-C architectures (UART vs Native OTG), Windows Arduino toolchain quirks, hardware reset (DTR/RTS) bootloader states, WS2812 multi-pin routing, Wi-Fi SoftAP, and Captive Portal networking.
---

# ESP32 & ESP32-S3 Embedded Development Guide

This guide contains essential engineering knowledge, physical hardware runbooks, and operational workflows for developing, flashing, and stabilizing ESP32 and ESP32-S3 firmware.

---

## 1. Physical Hardware & Recovery Runbook

### Physical Button Operations
Most ESP32-S3 development boards feature two physical tactile pushbuttons:
- **`EN` / `RST` (Reset):** Connected to the Enable pin. Pulling this to ground resets the chip.
- **`BOOT` (IO0):** Connected to GPIO 0. Pulling this to ground during boot tells the ROM to enter Download Bootloader mode.

---

### Boot Loop / Crash-On-Boot Recovery (Manual Bootloader Entry)
If buggy code, corrupted flash, memory panics, or bad SPIFFS data puts the ESP32 into a rapid reset loop:

1. **Press and HOLD the `BOOT` button.**
2. **Press and RELEASE the `EN` / `RST` button** (while still holding `BOOT`).
3. **RELEASE the `BOOT` button.**
4. Check the serial monitor (115200 baud). The output will display:
   ```text
   rst:0x1 (POWERON),boot:0x0 (DOWNLOAD(USB/UART0))
   waiting for download
   ```
5. **Flash New Firmware or Erase Flash:**
   The board will remain safely frozen in the ROM bootloader without executing the crashing sketch, allowing you to upload clean code:
   ```powershell
   # Upload firmware:
   & "C:\Program Files\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe" upload -p COM5 -b esp32:esp32:esp32s3:USBMode=default,CDCOnBoot=default -v <SketchPath>
   
   # Or completely wipe corrupted flash/NVS:
   & "C:\Users\<User>\AppData\Local\Arduino15\packages\esp32\tools\esptool_py\5.3.1\esptool.exe" --chip esp32s3 --port COM5 erase_flash
   ```
6. **Press the `EN` / `RST` button once** to reboot into the newly flashed firmware.

---

### Recovering from an Autorun / BadUSB Lockout
If an autorun DuckScript payload begins injecting unwanted keystrokes into your computer upon power-up:

1. **Unplug the Native USB-OTG port immediately.**
2. **Plug your cable ONLY into the UART / Programming Port:**
   - The UART bridge chip (CH340/CH343/CP2102) only carries serial data; it **cannot physically emit USB HID keystrokes** to your operating system.
3. Open a serial terminal (or PowerShell script) on the COM port at `115200` baud.
4. Send CLI reset commands:
   ```text
   set autorun ""
   save
   ```
   Or send `reset` to restore factory defaults.

---

### Brownout Detector & Power Issues
If the serial console prints:
```text
Brownout detector was triggered
```
or resets unexpectedly with `rst:0xc (RTC_SW_CPU_RST)` or `rst:0x10 (RTCWDT_RTC_RST)`:
- **Root Cause:** ESP32 Wi-Fi RF power bursts can draw up to 350-500 mA instantaneously. If your USB cable is long, high-gauge, or plugged into an unpowered USB hub, the 5V line sags below 4.5V, causing the onboard 3.3V LDO regulator to drop out.
- **Remedies:**
  - Connect directly to a motherboard USB 3.0 / USB-C port (avoid unpowered passive splitters).
  - Use a high-quality, short USB-C data cable.
  - Keep RGB LED brightness `<= 20-30` (full white on a NeoPixel adds ~60 mA continuous load).
  - On custom PCBs, place a 10µF to 100µF low-ESR decoupling capacitor near the ESP32 `3V3` and `GND` pins.

---

### Thermal Management & Automatic Throttling
- Operating temperature between **40°C and 55°C** is normal for an ESP32-S3 running dual 240 MHz cores with an active Wi-Fi SoftAP.
- **Built-in Thermal Throttling (`thermal.h` / `thermal.cpp`):**
  - Continuous on-die silicon sensor monitoring via `temperatureRead()`.
  - **Throttle Trigger (>= 70°C):**
    - Automatically dims RGB LED power to minimal level 5.
    - Throttles Wi-Fi RF power from 19.5 dBm down to 11 dBm (`WIFI_POWER_11dBm`).
    - Surfaces alert banner on the Web UI dashboard and flags `throttled: true` in `/status`.
  - **Hysteresis Recovery (<= 65°C):**
    - Automatically restores user-configured LED brightness and full Wi-Fi TX power once cooled.
- **CLI Check:**
  - Send `temp` in serial CLI to inspect live silicon temperature and throttle status:
    ```text
    ESP32-S3 Core Temp: 53.1 C (127.6 F) | Limit: 70.0 C | Status: NORMAL
    ```

---

### MicroSD Hardware Troubleshooting
If `SD card mount failed` appears in logs:
- **Filesystem Format:** Must be formatted as **FAT32** or **FAT16** with 32KB allocation size. **exFAT and NTFS are NOT supported** by the standard Arduino SD driver.
- **Card Capacity:** Use 2 GB to 32 GB MicroSD cards (SDHC). Cards >32 GB formatted as SDXC/exFAT will fail.
- **SPI Wire Lengths:** High-speed SPI (25 MHz) degrades over loose jumper wires > 10 cm. If using breadboard jumpers, reduce `SD_SPI_FREQ_HZ` to `10000000` (10 MHz) or `4000000` (4 MHz) in `config.h`.

---

## 2. Port Architecture (ESP32-S3)

### How to Physically Identify the Ports
1. **UART / Programming Port (`COM` / `UART`):**
   - Traced to a small rectangular interface IC (CH340, CH343, or CP2102) adjacent to the port.
   - Used for firmware flashing and serial monitoring (`115200` baud).
   - **Does NOT emit USB keyboard strokes.**
2. **Native USB-OTG Port (`USB` / `Native` / `OTG`):**
   - Traces run directly to the ESP32-S3 chip through series damping resistors to GPIO 19 (`D-`) and GPIO 20 (`D+`).
   - Acts as the USB Rubber Ducky keyboard emulator and USB Mass Storage device.
   - Requires compilation flag: `USBMode=default,CDCOnBoot=default`.

---

## 3. Hardware Reset & Serial Port Pitfall (Crucial)

### The DTR/RTS "Download Mode" Trap
ESP32 auto-programming circuits use DTR and RTS lines:
- `EN` is driven by `RTS & ~DTR`
- `IO0` is driven by `DTR & ~RTS`

> [!WARNING]
> Opening `COM` ports on Windows with default `DTR=true` pulls `IO0` LOW while releasing `EN`, trapping the chip in ROM bootloader mode:
> ```text
> rst:0x1 (POWERON),boot:0x0 (DOWNLOAD(USB/UART0))
> waiting for download
> ```
> In this state, user firmware is completely frozen (Wi-Fi, LEDs, HID all stopped).

### Safe Hardware Run-Reset Sequence (PowerShell)
To reboot the ESP32 cleanly into SPI Flash run mode (`boot:0x8`):

```powershell
$port = New-Object System.IO.Ports.SerialPort("COM5", 115200)
$port.DtrEnable = $false
$port.RtsEnable = $false
try {
    $port.Open()
    $port.DtrEnable = $false  # Keep IO0 HIGH (normal boot)
    $port.RtsEnable = $true   # Assert reset (EN LOW)
    Start-Sleep -Milliseconds 150
    $port.RtsEnable = $false  # Release reset (EN HIGH)
} finally {
    $port.DtrEnable = $false
    $port.RtsEnable = $false
    $port.Close()
}
```

---

## 4. Toolchain & Windows Compilation Quirks

### Arduino CLI Commands
- **Compile:**
  ```powershell
  & "C:\Program Files\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe" compile -b esp32:esp32:esp32s3:USBMode=default,CDCOnBoot=default <SketchDirectory>
  ```
- **Upload:**
  ```powershell
  & "C:\Program Files\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe" upload -p COM5 -b esp32:esp32:esp32s3:USBMode=default,CDCOnBoot=default -v <SketchDirectory>
  ```

### Windows Case-Insensitivity File Collision Bug
> [!IMPORTANT]
> On Windows filesystems, `#include <SPIFFS.h>` will resolve to a local file named `spiffs.h` or `spiffs.cpp` in the sketch directory instead of the ESP32 core filesystem header.
> - **Symptom:** Compilation error `'SPIFFS' was not declared in this scope`.
> - **Rule:** Always prefix custom wrappers (e.g. `duck_spiffs.h` and `duck_spiffs.cpp`) to avoid header collisions.

### Third-Party Library Dependencies
Standard Arduino library path: `C:\Users\<User>\Documents\Arduino\libraries\`
- `Adafruit NeoPixel`
- `AsyncTCP` (`ESP32Async/AsyncTCP`)
- `ESPAsyncWebServer` (`ESP32Async/ESPAsyncWebServer`)

---

## 5. Wi-Fi SoftAP & Captive Portal Best Practices

### 1. Explicit SoftAP IP Configuration
Always configure the subnet before starting SoftAP so DHCP leases resolve reliably:
```cpp
WiFi.mode(WIFI_AP);
WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
WiFi.softAP(ssid, password, channel, hidden, max_connection);
```

### 2. TX Power Ordering & Signal Strength
- In ESP32 Core 3.x, `WiFi.setTxPower()` returns `false` if called before the AP starts. Always set TX power **after** `WiFi.softAP()`:
  ```cpp
  WiFi.softAP(...);
  WiFi.setTxPower(WIFI_POWER_19_5dBm); // 78 = 19.5 dBm
  ```
- Avoid low TX power (< 15 dBm) on PCB trace antennas; smartphones will drop connection or fail DHCP negotiation.

### 3. Never Enable Modem Sleep on SoftAPs
```cpp
// AVOID:
WiFi.setSleep(true); // Causes beacon packet drops and phone disconnections
```

### 4. Asynchronous Captive Portal DNS
Smartphones (Android 10+, iOS 14+) require a Captive Portal DNS server to prevent falling back to mobile cellular data:
```cpp
#include <DNSServer.h>
static DNSServer dnsServer;

// In setup/begin:
dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
```

In the HTTP server `onNotFound` or static handler, redirect probe URLs with HTTP 302:
```cpp
if (path == "/generate_204" || path == "/gen_204" ||
    path == "/hotspot-detect.html" || path == "/canonical.html" ||
    path == "/connecttest.txt" || path == "/ncsi.txt") {
  req->redirect("http://192.168.4.1/");
  return;
}
```

---

## 6. WS2812 RGB LED Multi-Pin Driving

DevKit boards and custom production PCBs frequently route the WS2812 data line to different GPIOs (e.g., GPIO 48 on DevKits, GPIO 1 on custom boards):

```cpp
static const uint8_t LED_CANDIDATE_PINS[] = { 1, 48, 38, 21 };
static Adafruit_NeoPixel strip1(1, 1, NEO_GRB + NEO_KHZ800);
static Adafruit_NeoPixel strip48(1, 48, NEO_GRB + NEO_KHZ800);
static Adafruit_NeoPixel strip38(1, 38, NEO_GRB + NEO_KHZ800);
static Adafruit_NeoPixel strip21(1, 21, NEO_GRB + NEO_KHZ800);

void showColor(uint8_t r, uint8_t g, uint8_t b) {
  strip1.setPixelColor(0, strip1.Color(r, g, b));  strip1.show();
  strip48.setPixelColor(0, strip48.Color(r, g, b)); strip48.show();
  strip38.setPixelColor(0, strip38.Color(r, g, b)); strip38.show();
  strip21.setPixelColor(0, strip21.Color(r, g, b)); strip21.show();
}
```

---

## 7. USB HID Keystroke Debouncing

To eliminate swallowed keys or duplicate keystrokes across diverse operating systems (Windows, macOS, Linux), insert a 4ms USB debounce delay after key release:

```cpp
void releaseAll() {
  Keyboard.releaseAll();
  delay(4); // Debounce to allow USB host buffers to process report state change
}
```

---

## 8. Complete Verification Checklist

Before closing any ESP32 task:
- [ ] Code compiles with `arduino-cli compile` with 0 warnings/errors.
- [ ] Binary uploads successfully via `arduino-cli upload`.
- [ ] Board boots cleanly with `rst:0x1, boot:0x8` (NOT stuck in `boot:0x0`).
- [ ] Serial output confirms AP SSID and IP (`192.168.4.1`).
- [ ] Captive portal DNS is running on port 53.
- [ ] Phone can connect and reach `http://192.168.4.1`.
- [ ] If in boot loop: held `BOOT`, pressed `EN`, released `BOOT`, re-flashed cleanly.
- [ ] All changes committed to the active Git branch.
