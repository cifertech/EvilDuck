#pragma once

#include <Arduino.h>

namespace attackmode {
  void begin();
  bool hidEnabled();
  bool storageEnabled();
  void setHid(bool on);
  void setStorage(bool on);
  void setOff();
  void setHidOnly();
  void setStorageOnly();
  void setHidStorage();
  bool applyFromString(const String &modes);
}
