#include "hid_backend.h"
#include "debug.h"
#include "USB.h"
#include "USBHID.h"
#include "USBHIDKeyboard.h"

static USBHID HID;
static USBHIDKeyboard Keyboard;

namespace {
constexpr uint8_t HID_A = 0x04;
constexpr uint8_t HID_1 = 0x1E;
constexpr uint8_t HID_ENTER = 0x28;
constexpr uint8_t HID_ESCAPE = 0x29;
constexpr uint8_t HID_BACKSPACE = 0x2A;
constexpr uint8_t HID_TAB = 0x2B;
constexpr uint8_t HID_SPACE = 0x2C;
constexpr uint8_t HID_MINUS = 0x2D;
constexpr uint8_t HID_EQUAL = 0x2E;
constexpr uint8_t HID_LEFT_BRACE = 0x2F;
constexpr uint8_t HID_RIGHT_BRACE = 0x30;
constexpr uint8_t HID_BACKSLASH = 0x31;
constexpr uint8_t HID_SEMICOLON = 0x33;
constexpr uint8_t HID_APOSTROPHE = 0x34;
constexpr uint8_t HID_GRAVE = 0x35;
constexpr uint8_t HID_COMMA = 0x36;
constexpr uint8_t HID_DOT = 0x37;
constexpr uint8_t HID_SLASH = 0x38;
constexpr uint8_t HID_CAPSLOCK = 0x39;
constexpr uint8_t HID_DELETE = 0x4C;
constexpr uint8_t HID_INSERT = 0x49;
constexpr uint8_t HID_HOME = 0x4A;
constexpr uint8_t HID_PAGEUP = 0x4B;
constexpr uint8_t HID_END = 0x4D;
constexpr uint8_t HID_PAGEDOWN = 0x4E;
constexpr uint8_t HID_RIGHT = 0x4F;
constexpr uint8_t HID_LEFT = 0x50;
constexpr uint8_t HID_DOWN = 0x51;
constexpr uint8_t HID_UP = 0x52;
constexpr uint8_t HID_NUMLOCK = 0x53;
constexpr uint8_t HID_NUM_SLASH = 0x54;
constexpr uint8_t HID_NUM_ASTERISK = 0x55;
constexpr uint8_t HID_NUM_MINUS = 0x56;
constexpr uint8_t HID_NUM_PLUS = 0x57;
constexpr uint8_t HID_NUM_ENTER = 0x58;
constexpr uint8_t HID_NUM_1 = 0x59;
constexpr uint8_t HID_NUM_2 = 0x5A;
constexpr uint8_t HID_NUM_3 = 0x5B;
constexpr uint8_t HID_NUM_4 = 0x5C;
constexpr uint8_t HID_NUM_5 = 0x5D;
constexpr uint8_t HID_NUM_6 = 0x5E;
constexpr uint8_t HID_NUM_7 = 0x5F;
constexpr uint8_t HID_NUM_8 = 0x60;
constexpr uint8_t HID_NUM_9 = 0x61;
constexpr uint8_t HID_NUM_0 = 0x62;
constexpr uint8_t HID_NUM_DOT = 0x63;
constexpr uint8_t HID_APP = 0x65;
constexpr uint8_t HID_PRINTSCREEN = 0x46;
constexpr uint8_t HID_SCROLLLOCK = 0x47;
constexpr uint8_t HID_PAUSE = 0x48;

void applyModifiers(uint8_t modifiers) {
  if (modifiers & 0x01) Keyboard.pressRaw(0xE0);
  if (modifiers & 0x02) Keyboard.pressRaw(0xE1);
  if (modifiers & 0x04) Keyboard.pressRaw(0xE2);
  if (modifiers & 0x08) Keyboard.pressRaw(0xE3);
  if (modifiers & 0x10) Keyboard.pressRaw(0xE4);
  if (modifiers & 0x20) Keyboard.pressRaw(0xE5);
  if (modifiers & 0x40) Keyboard.pressRaw(0xE6);
  if (modifiers & 0x80) Keyboard.pressRaw(0xE7);
}
}

namespace hid_backend {

void begin() {
  HID.begin();
  Keyboard.begin();
  USB.begin();
  debug::info("USB HID started");
}

bool keyNameToHid(const String &input, uint8_t &outCode) {
  String key = input;
  key.trim();
  key.toUpperCase();

  if (key.length() == 1) {
    char c = key[0];
    if (c >= 'A' && c <= 'Z') {
      outCode = HID_A + static_cast<uint8_t>(c - 'A');
      return true;
    }
    if (c >= '1' && c <= '9') {
      outCode = HID_1 + static_cast<uint8_t>(c - '1');
      return true;
    }
    if (c == '0') { outCode = 0x27; return true; }
  }

  if (key == "ENTER" || key == "RETURN") { outCode = HID_ENTER; return true; }
  if (key == "ESC" || key == "ESCAPE") { outCode = HID_ESCAPE; return true; }
  if (key == "TAB") { outCode = HID_TAB; return true; }
  if (key == "SPACE") { outCode = HID_SPACE; return true; }
  if (key == "MINUS") { outCode = HID_MINUS; return true; }
  if (key == "EQUAL") { outCode = HID_EQUAL; return true; }
  if (key == "LBRACE" || key == "LEFTBRACE") { outCode = HID_LEFT_BRACE; return true; }
  if (key == "RBRACE" || key == "RIGHTBRACE") { outCode = HID_RIGHT_BRACE; return true; }
  if (key == "BACKSLASH") { outCode = HID_BACKSLASH; return true; }
  if (key == "SEMICOLON") { outCode = HID_SEMICOLON; return true; }
  if (key == "APOSTROPHE" || key == "QUOTE") { outCode = HID_APOSTROPHE; return true; }
  if (key == "GRAVE" || key == "BACKTICK") { outCode = HID_GRAVE; return true; }
  if (key == "COMMA") { outCode = HID_COMMA; return true; }
  if (key == "DOT" || key == "PERIOD") { outCode = HID_DOT; return true; }
  if (key == "SLASH") { outCode = HID_SLASH; return true; }
  if (key == "BACKSPACE") { outCode = HID_BACKSPACE; return true; }
  if (key == "INSERT") { outCode = HID_INSERT; return true; }
  if (key == "HOME") { outCode = HID_HOME; return true; }
  if (key == "PAGEUP") { outCode = HID_PAGEUP; return true; }
  if (key == "PAGEDOWN") { outCode = HID_PAGEDOWN; return true; }
  if (key == "END") { outCode = HID_END; return true; }
  if (key == "DELETE" || key == "DEL") { outCode = HID_DELETE; return true; }
  if (key == "UP" || key == "UPARROW") { outCode = HID_UP; return true; }
  if (key == "DOWN" || key == "DOWNARROW") { outCode = HID_DOWN; return true; }
  if (key == "LEFT" || key == "LEFTARROW") { outCode = HID_LEFT; return true; }
  if (key == "RIGHT" || key == "RIGHTARROW") { outCode = HID_RIGHT; return true; }
  if (key == "MENU" || key == "APP") { outCode = HID_APP; return true; }
  if (key == "PAUSE" || key == "BREAK") { outCode = HID_PAUSE; return true; }
  if (key == "CAPSLOCK") { outCode = HID_CAPSLOCK; return true; }
  if (key == "NUMLOCK") { outCode = HID_NUMLOCK; return true; }
  if (key == "PRINTSCREEN") { outCode = HID_PRINTSCREEN; return true; }
  if (key == "SCROLLLOCK") { outCode = HID_SCROLLLOCK; return true; }
  if (key == "NUM_ASTERIX" || key == "NUM_ASTERISK") { outCode = HID_NUM_ASTERISK; return true; }
  if (key == "NUM_ENTER") { outCode = HID_NUM_ENTER; return true; }
  if (key == "NUM_MINUS") { outCode = HID_NUM_MINUS; return true; }
  if (key == "NUM_DOT") { outCode = HID_NUM_DOT; return true; }
  if (key == "NUM_PLUS") { outCode = HID_NUM_PLUS; return true; }
  if (key == "NUM_SLASH") { outCode = HID_NUM_SLASH; return true; }
  if (key == "NUM_0") { outCode = HID_NUM_0; return true; }
  if (key == "NUM_1") { outCode = HID_NUM_1; return true; }
  if (key == "NUM_2") { outCode = HID_NUM_2; return true; }
  if (key == "NUM_3") { outCode = HID_NUM_3; return true; }
  if (key == "NUM_4") { outCode = HID_NUM_4; return true; }
  if (key == "NUM_5") { outCode = HID_NUM_5; return true; }
  if (key == "NUM_6") { outCode = HID_NUM_6; return true; }
  if (key == "NUM_7") { outCode = HID_NUM_7; return true; }
  if (key == "NUM_8") { outCode = HID_NUM_8; return true; }
  if (key == "NUM_9") { outCode = HID_NUM_9; return true; }

  if (key.startsWith("F")) {
    int n = key.substring(1).toInt();
    if (n >= 1 && n <= 24) {
      outCode = static_cast<uint8_t>(0x3A + (n - 1));
      return true;
    }
  }
  return false;
}

void pressModifier(uint8_t modMask) {
  applyModifiers(modMask);
}

void pressKeyHeld(uint8_t hidKeycode) {
  Keyboard.pressRaw(hidKeycode);
}

void pressReport(uint8_t modifiers, const uint8_t *keys, uint8_t keyCount) {
  applyModifiers(modifiers);
  for (uint8_t i = 0; i < keyCount && i < 6; ++i) {
    if (keys[i]) Keyboard.pressRaw(keys[i]);
  }
  delay(5);
}

void pressKeycode(uint8_t hidKeycode, uint8_t modifiers) {
  pressReport(modifiers, &hidKeycode, hidKeycode ? 1 : 0);
  releaseAll();
}

bool pressNamedCombo(const String &keyName, uint8_t modifiers) {
  uint8_t keycode = 0;
  if (!keyNameToHid(keyName, keycode)) return false;
  pressKeycode(keycode, modifiers);
  return true;
}

void releaseAll() {
  Keyboard.releaseAll();
  delay(4);
}

void sendText(const String &text) {
  Keyboard.print(text);
}

void sendTextLn(const String &text) {
  Keyboard.print(text);
  pressEnter();
}

void pressEnter() { pressKeycode(HID_ENTER, 0); }
void pressTab() { pressKeycode(HID_TAB, 0); }
void pressEscape() { pressKeycode(HID_ESCAPE, 0); }
void pressSpace() { pressKeycode(HID_SPACE, 0); }
void pressBackspace() { pressKeycode(HID_BACKSPACE, 0); }
void pressDelete() { pressKeycode(HID_DELETE, 0); }
void pressArrowUp() { pressKeycode(HID_UP, 0); }
void pressArrowDown() { pressKeycode(HID_DOWN, 0); }
void pressArrowLeft() { pressKeycode(HID_LEFT, 0); }
void pressArrowRight() { pressKeycode(HID_RIGHT, 0); }

}
