#include "keyboard_locale.h"
#include "hid_backend.h"
#include "locale/locales.h"
#include <pgmspace.h>

static hid_locale_t *activeLocale = &locale_us;

namespace keyboard_locale {

void begin() {
  activeLocale = &locale_us;
}

void setLocale(hid_locale_t *locale) {
  if (locale) activeLocale = locale;
}

hid_locale_t *getLocale() {
  return activeLocale;
}

static bool nameEq(const String &a, const char *b) {
  String x = a;
  x.toUpperCase();
  return x == b;
}

bool setLocaleByName(const String &name) {
  if (nameEq(name, "US")) { setLocale(&locale_us); return true; }
  if (nameEq(name, "DE")) { setLocale(&locale_de); return true; }
#if defined(HAS_LOCALE_GB)
  if (nameEq(name, "GB")) { setLocale(&locale_gb); return true; }
#endif
#if defined(HAS_LOCALE_ES)
  if (nameEq(name, "ES")) { setLocale(&locale_es); return true; }
#endif
#if defined(HAS_LOCALE_FR)
  if (nameEq(name, "FR")) { setLocale(&locale_fr); return true; }
#endif
#if defined(HAS_LOCALE_RU)
  if (nameEq(name, "RU")) { setLocale(&locale_ru); return true; }
#endif
  return false;
}

static uint8_t pressChar(const uint8_t *b) {
  for (size_t i = 0; i < activeLocale->combinations_len; ++i) {
    uint8_t res = 0;
    for (uint8_t j = 0; j < 4; ++j) {
      uint8_t keyCode = pgm_read_byte(activeLocale->combinations + (i * 8) + j);
      if (keyCode == 0) break;
      if (keyCode == b[j]) ++res;
      else { res = 0; break; }
    }
    if (res > 0) {
      uint8_t m1 = pgm_read_byte(activeLocale->combinations + (i * 8) + 4);
      uint8_t k1 = pgm_read_byte(activeLocale->combinations + (i * 8) + 5);
      uint8_t m2 = pgm_read_byte(activeLocale->combinations + (i * 8) + 6);
      uint8_t k2 = pgm_read_byte(activeLocale->combinations + (i * 8) + 7);
      hid_backend::pressKeycode(k1, m1);
      hid_backend::releaseAll();
      hid_backend::pressKeycode(k2, m2);
      hid_backend::releaseAll();
      return res - 1;
    }
  }

  if (b[0] < activeLocale->ascii_len) {
    uint8_t mod = pgm_read_byte(activeLocale->ascii + (b[0] * 2));
    uint8_t key = pgm_read_byte(activeLocale->ascii + (b[0] * 2) + 1);
    if (key != 0) {
      hid_backend::pressKeycode(key, mod);
      hid_backend::releaseAll();
    }
    return 0;
  }

  for (size_t i = 0; i < activeLocale->utf8_len; ++i) {
    uint8_t res = 0;
    for (uint8_t j = 0; j < 4; ++j) {
      uint8_t keyCode = pgm_read_byte(activeLocale->utf8 + (i * 6) + j);
      if (keyCode == 0) break;
      if (keyCode == b[j]) ++res;
      else { res = 0; break; }
    }
    if (res > 0) {
      uint8_t mod = pgm_read_byte(activeLocale->utf8 + (i * 6) + 4);
      uint8_t key = pgm_read_byte(activeLocale->utf8 + (i * 6) + 5);
      hid_backend::pressKeycode(key, mod);
      hid_backend::releaseAll();
      return res - 1;
    }
  }
  return 0;
}

void write(const char *str, size_t len) {
  for (size_t i = 0; i < len; ) {
    i += 1 + pressChar(reinterpret_cast<const uint8_t *>(str + i));
  }
}

void write(const String &text) {
  write(text.c_str(), text.length());
}

}
