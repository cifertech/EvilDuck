#pragma once
#include <Arduino.h>

namespace script_log {
  void clear();
  void info(const String &msg);
  void warn(const String &msg);
  void error(const String &msg);
  void setLine(uint32_t line, const String &scriptName);
  String getJson();
  String getText();
}
