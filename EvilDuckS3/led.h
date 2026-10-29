#pragma once
#include <Arduino.h>

namespace led {
  void begin();
  void setColor(int r, int g, int b);
  void rainbowCycle(uint16_t totalDurationMs = 1200);
  bool enabled();
}
