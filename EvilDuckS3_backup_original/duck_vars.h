#pragma once
#include <Arduino.h>

namespace duck_vars {
  void clear();
  bool set(const String &name, const String &value);
  bool get(const String &name, String &out);
  String expand(const String &input);
  bool truthy(const String &name);
}
