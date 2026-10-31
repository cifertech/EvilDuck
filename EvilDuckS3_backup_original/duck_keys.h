#pragma once

#include <Arduino.h>

namespace duck_keys {
  bool isModifierName(const String &token, uint8_t &modMask);
  bool tokenToHid(const String &token, uint8_t &hidCode);
  void pressLineTokens(const String &line);
}
