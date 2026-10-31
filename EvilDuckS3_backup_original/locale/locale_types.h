#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct hid_locale_t {
  uint8_t *ascii;
  uint8_t ascii_len;
  uint8_t *utf8;
  size_t utf8_len;
  uint8_t *combinations;
  size_t combinations_len;
} hid_locale_t;
