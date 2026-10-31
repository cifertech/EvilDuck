#include <Arduino.h>
#include "locale/locale_us.h"
#include "locale/locale_de.h"

hid_locale_t locale_us = {
  (uint8_t *)ascii_us, 128,
  (uint8_t *)utf8_us, 0,
  (uint8_t *)combinations_us, sizeof(combinations_us) / 8,
};

hid_locale_t locale_de = {
  (uint8_t *)ascii_de, 128,
  (uint8_t *)utf8_de, sizeof(utf8_de) / 6,
  (uint8_t *)combinations_de, sizeof(combinations_de) / 8,
};
