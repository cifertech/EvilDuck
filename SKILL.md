---
name: esp32-embedded-development
description: Comprehensive operational runbook and engineering guidelines for building, debugging, flashing, and stabilizing ESP32 and ESP32-S3 firmware projects (including EvilDuck). Covers dual USB-C architectures (UART vs Native OTG), Windows Arduino toolchain quirks, hardware reset (DTR/RTS) bootloader states, WS2812 multi-pin routing, Wi-Fi SoftAP, and Captive Portal networking.
---

# ESP32 & ESP32-S3 Embedded Development Guide

This guide contains essential engineering knowledge, troubleshooting workflows, and operational runbooks for developing, flashing, and debugging ESP32 and ESP32-S3 firmware.

---

## 1. Hardware & Port Architecture (ESP32-S3)

### Dual USB-C Ports
ESP32-S3 development boards and custom hardware typically feature **two USB-C ports**:
1. **UART / Programming Port (CH340 / CH343 / CP2102):**
   - Connected to hardware UART0 (`TX0` = GPIO 43, `RX0` = GPIO 44).
   - Used for firmware uploads (`arduino-cli upload`), serial logging, and bootloader recovery.
   - Baud rate: `115200` (CLI / Debug), `921600` (Flashing).
   - **Does NOT emit USB HID / BadUSB keystrokes.**
2. **Native USB-OTG Port (Direct SoC USB):**
   - Connected directly to ESP32-S3 native pins (`GPIO 19` = D-, `GPIO 20` = D+).
   - Required for USB HID injection (Rubber Ducky emulation), USB Mass Storage, and WebUSB.
   - Must be compiled with native TinyUSB support (`USBMode=default,CDCOnBoot=default`).

---

## 2. Hardware Reset & Serial Port Pitfall (Crucial)

### The DTR/RTS "Download Mode" Trap
ESP32 auto-programming circuits use DTR and RTS lines to control `EN` (Reset) and `IO0` (Boot Mode):
- `EN` is driven by `RTS & ~DTR`
- `IO0` is driven by `DTR & ~RTS`

> [!WARNING]
> Opening `COM` ports on Windows with default `DTR=true` pulls `IO0` LOW while releasing `EN`, trapping the chip in ROM bootloader mode:
> ```text
> rst:0x1 (POWERON),boot:0x0 (DOWNLOAD(USB/UART0))
> waiting for download
> ```
> While in this state, **no user firmware runs**: Wi-Fi is OFF, LEDs are OFF, and USB HID is inactive.

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

## 3. Toolchain & Windows Compilation Quirks

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

## 4. Wi-Fi SoftAP & Captive Portal Best Practices

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

## 5. WS2812 RGB LED Multi-Pin Driving

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
> [!TIP]
> Keep default brightness low (`20` out of `255`) to eliminate thermal heating on compact boards.

---

## 6. USB HID Keystroke Debouncing

To eliminate swallowed keys or duplicate keystrokes across diverse operating systems (Windows, macOS, Linux), insert a 4ms USB debounce delay after key release:

```cpp
void releaseAll() {
  Keyboard.releaseAll();
  delay(4); // Debounce to allow USB host buffers to process report state change
}
```

---

## 7. Verification Checklist

Before closing any ESP32 task:
- [ ] Code compiles with `arduino-cli compile` with 0 warnings/errors.
- [ ] Binary uploads successfully via `arduino-cli upload`.
- [ ] Board boots cleanly with `rst:0x1, boot:0x8` (NOT stuck in `boot:0x0`).
- [ ] Serial output confirms AP SSID and IP (`192.168.4.1`).
- [ ] Captive portal DNS is running on port 53.
- [ ] Phone can connect and reach `http://192.168.4.1`.
- [ ] All changes committed to the active Git branch.
