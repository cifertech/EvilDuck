#pragma once

#ifndef WIFI_SSID
#define WIFI_SSID "EvilDuck_S3"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "12345678"
#endif

#define SPIFFS_BASE_PATH "/"
#define DUCKSCRIPT_MAX_LINE_LEN 512
#define DEBUG_SERIAL_BAUD 115200

// ===== Status LED (WS2812 / NeoPixel) =====
// Custom EvilDuckS3 PCB routes the NeoPixel data line to GPIO1.
#define LED_ENABLED 1
#define LED_TYPE_NEOPIXEL 1
#define LED_PIN 1
#define LED_NUM_PIXELS 1
#define LED_BRIGHTNESS 120
// WS2812-2020 packages commonly use RGB order. If pure colors are swapped,
// test NEO_GRB, NEO_RGB, NEO_BRG, NEO_BGR, NEO_RBG, or NEO_GBR.
#define LED_NEOPIXEL_ORDER NEO_GRB
// Per-channel correction, 0-255.
#define LED_CORRECT_R 255
#define LED_CORRECT_G 255
#define LED_CORRECT_B 255
#define LED_USE_GAMMA 0
#define LED_PIN_R 25
#define LED_PIN_G 26
#define LED_PIN_B 27
#define LED_ANODE 0

// ===== microSD (SPI) — set SD_ENABLED 1 when wired =====
#define SD_ENABLED 38
#define SD_CS   10
#define SD_MOSI 11
#define SD_MISO 13
#define SD_SCK  12
#define SD_SPI_FREQ_HZ 25000000
