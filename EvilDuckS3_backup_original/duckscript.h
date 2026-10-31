#pragma once
#include <Arduino.h>

namespace duckscript {
  bool run(const String &filename);
  void stop();
  void update();
  bool running();
  String runningScript();
  uint32_t currentLine();
  String lastError();
}
