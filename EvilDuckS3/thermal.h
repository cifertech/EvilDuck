#pragma once
#include <Arduino.h>

namespace thermal {
  void begin();
  void update();
  float getTemp();
  bool isThrottled();
  float getThreshold();
}
