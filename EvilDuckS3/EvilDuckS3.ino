#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "debug.h"
#include "storage.h"
#include "settings.h"
#include "webserver.h"
#include "duckscript.h"
#include "com.h"
#include "cli.h"
#include "hid_backend.h"
#include "keyboard_locale.h"
#include "led.h"
#include "attackmode.h"
#include "script_library.h"

static void applyIdleLedColor() {
  if (settings::getDisableLed()) {
    led::setColor(0, 0, 0);
    return;
  }
  uint32_t color = settings::getLedColor();
  if (color == 0x000028) {
    color = 0x00FF88; // Vibrant emerald / teal duck color
  }
  led::setColor((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
}

void setup() {
  debug::begin();
  debug::info("Booting EvilDuck...");

  led::begin();
  // Vibrant RGB rainbow cycle on plug-in:
  led::rainbowCycle(1000);

  hid_backend::begin();
  keyboard_locale::begin();
  attackmode::begin();

  storage::begin();
  script_library::installIfNeeded();
  settings::load();
  applyIdleLedColor();
  cli::begin();

  webserver::begin();

  if (settings::autorunEnabled()) {
    String s = settings::getAutorun();
    if (s.length()) {
      uint32_t delayMs = settings::pickAutorunDelayMs();
      if (delayMs == 0) delayMs = settings::getStealthMode() ? 8000 : 3000;
      debug::info("Autorun in " + String(delayMs) + "ms: " + s);
      if (!settings::getDisableLed() && !settings::getStealthMode()) {
        led::setColor(255, 140, 0); // Amber warning during autorun countdown
      }
      delay(delayMs);
      duckscript::run(s);
    }
  }
}

void loop() {
  duckscript::update();
  com::update();
  cli::update();
  delay(1);
}
