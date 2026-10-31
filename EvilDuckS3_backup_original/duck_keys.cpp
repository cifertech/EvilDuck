#include "duck_keys.h"
#include "hid_backend.h"
#include "keyboard_locale.h"
#include "locale/usb_hid_keys.h"

namespace duck_keys {

bool isModifierName(const String &token, uint8_t &modMask) {
  String up = token;
  up.toUpperCase();
  if (up == "CTRL" || up == "CONTROL") { modMask = KEY_MOD_LCTRL; return true; }
  if (up == "SHIFT") { modMask = KEY_MOD_LSHIFT; return true; }
  if (up == "ALT") { modMask = KEY_MOD_LALT; return true; }
  if (up == "GUI" || up == "WINDOWS" || up == "WIN" || up == "CMD" || up == "COMMAND") {
    modMask = KEY_MOD_LMETA;
    return true;
  }
  if (up == "RIGHTCTRL" || up == "RCTRL") { modMask = KEY_MOD_RCTRL; return true; }
  if (up == "RIGHTSHIFT" || up == "RSHIFT") { modMask = KEY_MOD_RSHIFT; return true; }
  if (up == "RIGHTALT" || up == "RALT") { modMask = KEY_MOD_RALT; return true; }
  if (up == "RIGHTGUI" || up == "RIGHTWINDOWS") { modMask = KEY_MOD_RMETA; return true; }
  return false;
}

bool tokenToHid(const String &token, uint8_t &hidCode) {
  return hid_backend::keyNameToHid(token, hidCode);
}

void pressLineTokens(const String &line) {
  uint8_t modifiers = 0;
  uint8_t keys[6] = {};
  uint8_t keyCount = 0;

  int i = 0;
  const int n = line.length();
  while (i < n) {
    while (i < n && line[i] == ' ') ++i;
    if (i >= n) break;
    int j = i;
    while (j < n && line[j] != ' ') ++j;
    String tok = line.substring(i, j);
    i = j;

    uint8_t mod = 0;
    if (isModifierName(tok, mod)) {
      modifiers |= mod;
      continue;
    }
    uint8_t key = 0;
    if (tokenToHid(tok, key)) {
      if (keyCount < 6) keys[keyCount++] = key;
      continue;
    }
    if (modifiers || keyCount) {
      hid_backend::pressReport(modifiers, keys, keyCount);
      hid_backend::releaseAll();
      modifiers = 0;
      keyCount = 0;
    }
    keyboard_locale::write(tok);
  }

  if (modifiers || keyCount) {
    hid_backend::pressReport(modifiers, keys, keyCount);
    hid_backend::releaseAll();
  }
}

}
