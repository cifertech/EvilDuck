#pragma once
#include <Arduino.h>

namespace script_library {
  void installIfNeeded();
  String listJson();
  String listSpiffsOnlyJson();
  String listCombinedJson();
  bool readFirmware(const String &name, String &out);
  bool readExample(const String &name, String &out);
}
