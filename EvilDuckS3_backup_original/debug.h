#pragma once
#include <Arduino.h>

namespace debug {
  void begin();
  void info(const String &msg);
  void error(const String &msg);
}
