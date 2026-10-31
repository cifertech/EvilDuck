#include "thermal.h"
#include "debug.h"
#include "led.h"
#include "settings.h"
#include <WiFi.h>

namespace thermal {

static bool throttled = false;
static float currentTemp = 0.0f;
static uint32_t lastCheckMs = 0;
static const float THROTTLE_THRESHOLD = 70.0f;
static const float RESTORE_THRESHOLD = 65.0f;

void begin() {
  currentTemp = temperatureRead();
  if (isnan(currentTemp)) currentTemp = 0.0f;
}

float getTemp() {
  return currentTemp;
}

bool isThrottled() {
  return throttled;
}

float getThreshold() {
  return THROTTLE_THRESHOLD;
}

void update() {
  uint32_t now = millis();
  if (now - lastCheckMs < 2000) return;
  lastCheckMs = now;

  float t = temperatureRead();
  if (isnan(t)) return;
  currentTemp = t;

  if (!throttled && currentTemp >= THROTTLE_THRESHOLD) {
    throttled = true;
    debug::error("Thermal throttle TRIGGERED! Core temp: " + String(currentTemp, 1) + " C (>= 70 C). Reducing LED & Wi-Fi RF power.");
    led::setBrightness(5); // Throttle LED to minimal power
    WiFi.setTxPower(WIFI_POWER_11dBm); // Throttle RF transmission to cool down
  } else if (throttled && currentTemp <= RESTORE_THRESHOLD) {
    throttled = false;
    debug::info("Thermal throttle CLEARED. Core temp cooled to " + String(currentTemp, 1) + " C. Restoring normal power.");
    led::setBrightness(settings::getLedBrightness());
    WiFi.setTxPower((wifi_power_t)settings::getWifiPower());
  }
}

}
