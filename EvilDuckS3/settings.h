#pragma once
#include <Arduino.h>

namespace settings {
  void load();
  void save();
  void reset();

  bool autorunEnabled();
  String getAutorun();
  void setAutorun(const String &scriptName);

  String getSSID();
  String getPassword();
  uint8_t getChannel();
  void setSSID(const String &ssid);
  void setPassword(const String &password);
  void setChannel(uint8_t channel);

  bool getStealthMode();
  void setStealthMode(bool on);
  bool getHiddenAp();
  void setHiddenAp(bool on);
  bool getDisableLed();
  void setDisableLed(bool on);
  uint8_t getLedBrightness();
  void setLedBrightness(uint8_t brightness);
  uint32_t getLedColor();
  String getLedColorHex();
  void setLedColor(uint32_t color);
  bool setLedColorHex(const String &hex);
  String getWebPassword();
  void setWebPassword(const String &pw);
  uint16_t getAutorunDelayMin();
  uint16_t getAutorunDelayMax();
  void setAutorunDelay(uint16_t minMs, uint16_t maxMs);
  uint32_t pickAutorunDelayMs();

  // Wi-Fi Power & control
  uint8_t getWifiPower();
  void setWifiPower(uint8_t powerDb);
  String getWifiPowerLevel();
  void setWifiPowerLevel(const String &level);
  bool getWifiEnabled();
  void setWifiEnabled(bool on);

  // USB Keyboard Mode (BOOT = BIOS boot protocol, NORMAL = standard HID)
  String getUsbMode();
  void setUsbMode(const String &mode);
}
