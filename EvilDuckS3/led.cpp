#include "led.h"
#include "config.h"

#if LED_ENABLED

#if LED_TYPE_NEOPIXEL
#include <Adafruit_NeoPixel.h>
#ifndef LED_NEOPIXEL_ORDER
#define LED_NEOPIXEL_ORDER NEO_GRB
#endif
#ifndef LED_CORRECT_R
#define LED_CORRECT_R 255
#endif
#ifndef LED_CORRECT_G
#define LED_CORRECT_G 255
#endif
#ifndef LED_CORRECT_B
#define LED_CORRECT_B 255
#endif
#ifndef LED_USE_GAMMA
#define LED_USE_GAMMA 0
#endif

// Drive candidate WS2812 pins: EvilDuck PCB (GPIO 1), ESP32-S3 DevKit (GPIO 48), Waveshare (GPIO 38, 21)
static const uint8_t PINS[] = { LED_PIN, 48, 38, 21 };
static const size_t NUM_PINS = sizeof(PINS) / sizeof(PINS[0]);

static Adafruit_NeoPixel strip0(LED_NUM_PIXELS, PINS[0], LED_NEOPIXEL_ORDER + NEO_KHZ800);
static Adafruit_NeoPixel strip1(LED_NUM_PIXELS, PINS[1], LED_NEOPIXEL_ORDER + NEO_KHZ800);
static Adafruit_NeoPixel strip2(LED_NUM_PIXELS, PINS[2], LED_NEOPIXEL_ORDER + NEO_KHZ800);
static Adafruit_NeoPixel strip3(LED_NUM_PIXELS, PINS[3], LED_NEOPIXEL_ORDER + NEO_KHZ800);

static Adafruit_NeoPixel* strips[] = { &strip0, &strip1, &strip2, &strip3 };

static uint8_t corrected(uint8_t value, uint8_t scale) {
  uint16_t v = (static_cast<uint16_t>(value) * scale) / 255;
#if LED_USE_GAMMA
  return Adafruit_NeoPixel::gamma8(static_cast<uint8_t>(v));
#else
  return static_cast<uint8_t>(v);
#endif
}

namespace led {

void begin() {
  for (size_t p = 0; p < NUM_PINS; ++p) {
    strips[p]->begin();
    strips[p]->setBrightness(LED_BRIGHTNESS);
    strips[p]->show();
  }
}

void setColor(int r, int g, int b) {
  uint8_t rr = corrected(constrain(r, 0, 255), LED_CORRECT_R);
  uint8_t gg = corrected(constrain(g, 0, 255), LED_CORRECT_G);
  uint8_t bb = corrected(constrain(b, 0, 255), LED_CORRECT_B);
  for (size_t p = 0; p < NUM_PINS; ++p) {
    for (uint8_t i = 0; i < LED_NUM_PIXELS; ++i) {
      strips[p]->setPixelColor(i, strips[p]->Color(rr, gg, bb));
    }
    strips[p]->show();
  }
}

void rainbowCycle(uint16_t totalDurationMs) {
  int steps = 60;
  int delayPerStep = totalDurationMs / steps;
  if (delayPerStep < 5) delayPerStep = 5;
  for (int step = 0; step < steps; ++step) {
    uint16_t hue = (step * 65535UL) / steps;
    for (size_t p = 0; p < NUM_PINS; ++p) {
      uint32_t rgbcolor = strips[p]->gamma32(strips[p]->ColorHSV(hue));
      for (uint8_t i = 0; i < LED_NUM_PIXELS; ++i) {
        strips[p]->setPixelColor(i, rgbcolor);
      }
      strips[p]->show();
    }
    delay(delayPerStep);
  }
}

bool enabled() { return true; }

} // namespace led

#else
namespace led {
void begin() {
  pinMode(LED_PIN_R, OUTPUT);
  pinMode(LED_PIN_G, OUTPUT);
  pinMode(LED_PIN_B, OUTPUT);
}
void setColor(int r, int g, int b) {
#if LED_ANODE
  r = 255 - r; g = 255 - g; b = 255 - b;
#endif
  analogWrite(LED_PIN_R, r);
  analogWrite(LED_PIN_G, g);
  analogWrite(LED_PIN_B, b);
}
void rainbowCycle(uint16_t) {}
bool enabled() { return true; }
}
#endif

#else

namespace led {
void begin() {}
void setColor(int, int, int) {}
void rainbowCycle(uint16_t) {}
bool enabled() { return false; }
}

#endif
