#pragma once

#include <Arduino.h>
#include "locale/locale_types.h"

namespace keyboard_locale {
  void begin();
  void setLocale(hid_locale_t *locale);
  hid_locale_t *getLocale();
  bool setLocaleByName(const String &name);
  void write(const char *str, size_t len);
  void write(const String &text);
}
