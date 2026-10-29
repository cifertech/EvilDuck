#include "settings.h"
#include "debug.h"
#include "config.h"
#include <Preferences.h>

static Preferences prefs;
static bool autorun_enabled = false;
static String autorun_script_name = "";
static String ap_ssid = WIFI_SSID;
static String ap_password = WIFI_PASSWORD;
static uint8_t ap_channel = 1;

static bool stealth_mode = false;
static bool hidden_ap = false;
static bool disable_led = false;
static uint8_t led_brightness = LED_BRIGHTNESS;
static uint32_t led_color = 0x00FF88;
static String web_password = "";
static uint16_t autorun_delay_min = 0;
static uint16_t autorun_delay_max = 0;

static uint8_t wifi_power = 44; // 11 dBm default (WIFI_POWER_11dBm)
static bool wifi_enabled = true;

namespace settings {

void load() {
  prefs.begin("duck", true);
  autorun_enabled = prefs.getBool("autorun_enabled", false);
  autorun_script_name = prefs.getString("autorun_script", "");
  ap_ssid = prefs.getString("ap_ssid", WIFI_SSID);
  ap_password = prefs.getString("ap_password", WIFI_PASSWORD);
  ap_channel = static_cast<uint8_t>(prefs.getUChar("ap_channel", 1));
  stealth_mode = prefs.getBool("stealth", false);
  hidden_ap = prefs.getBool("hidden_ap", false);
  disable_led = prefs.getBool("disable_led", false);
  led_brightness = static_cast<uint8_t>(prefs.getUChar("led_bright", LED_BRIGHTNESS));
  led_color = prefs.getUInt("led_color", 0x00FF88);
  web_password = prefs.getString("web_pw", "");
  autorun_delay_min = prefs.getUShort("ar_dly_min", 0);
  autorun_delay_max = prefs.getUShort("ar_dly_max", 0);
  wifi_power = static_cast<uint8_t>(prefs.getUChar("wifi_power", 44));
  wifi_enabled = prefs.getBool("wifi_en", true);
  prefs.end();

  if (ap_ssid.length() == 0) ap_ssid = WIFI_SSID;
  if (ap_password.length() < 8) ap_password = WIFI_PASSWORD;
  if (ap_channel < 1 || ap_channel > 13) ap_channel = 1;
}

void save() {
  prefs.begin("duck", false);
  prefs.putBool("autorun_enabled", autorun_enabled);
  prefs.putString("autorun_script", autorun_script_name);
  prefs.putString("ap_ssid", ap_ssid);
  prefs.putString("ap_password", ap_password);
  prefs.putUChar("ap_channel", ap_channel);
  prefs.putBool("stealth", stealth_mode);
  prefs.putBool("hidden_ap", hidden_ap);
  prefs.putBool("disable_led", disable_led);
  prefs.putUChar("led_bright", led_brightness);
  prefs.putUInt("led_color", led_color);
  prefs.putString("web_pw", web_password);
  prefs.putUShort("ar_dly_min", autorun_delay_min);
  prefs.putUShort("ar_dly_max", autorun_delay_max);
  prefs.putUChar("wifi_power", wifi_power);
  prefs.putBool("wifi_en", wifi_enabled);
  prefs.end();
}

void reset() {
  autorun_enabled = false;
  autorun_script_name = "";
  ap_ssid = WIFI_SSID;
  ap_password = WIFI_PASSWORD;
  ap_channel = 1;
  stealth_mode = false;
  hidden_ap = false;
  disable_led = false;
  led_brightness = LED_BRIGHTNESS;
  led_color = 0x00FF88;
  web_password = "";
  autorun_delay_min = 0;
  autorun_delay_max = 0;
  wifi_power = 44;
  wifi_enabled = true;
  save();
}

bool autorunEnabled() { return autorun_enabled; }
String getAutorun() { return autorun_script_name; }
void setAutorun(const String &scriptName) {
  autorun_script_name = scriptName;
  autorun_enabled = scriptName.length() > 0;
}

String getSSID() { return ap_ssid; }
String getPassword() { return ap_password; }
uint8_t getChannel() { return ap_channel; }

void setSSID(const String &ssid) {
  if (ssid.length() > 0 && ssid.length() <= 32) ap_ssid = ssid;
}
void setPassword(const String &password) {
  if (password.length() >= 8 && password.length() <= 63) ap_password = password;
}
void setChannel(uint8_t channel) {
  if (channel >= 1 && channel <= 13) ap_channel = channel;
}

bool getStealthMode() { return stealth_mode; }
void setStealthMode(bool on) { stealth_mode = on; }
bool getHiddenAp() { return hidden_ap; }
void setHiddenAp(bool on) { hidden_ap = on; }
bool getDisableLed() { return disable_led; }
void setDisableLed(bool on) { disable_led = on; }

uint8_t getLedBrightness() { return led_brightness; }
void setLedBrightness(uint8_t brightness) { led_brightness = brightness; }

uint32_t getLedColor() { return led_color; }
String getLedColorHex() {
  char buf[8];
  snprintf(buf, sizeof(buf), "#%06X", static_cast<unsigned int>(led_color & 0xFFFFFF));
  return String(buf);
}
void setLedColor(uint32_t color) { led_color = color & 0xFFFFFF; }
bool setLedColorHex(const String &hex) {
  String h = hex;
  h.trim();
  if (h.startsWith("#")) h = h.substring(1);
  if (h.length() != 6) return false;
  uint32_t color = 0;
  for (uint8_t i = 0; i < 6; ++i) {
    char c = h[i];
    uint8_t v;
    if (c >= '0' && c <= '9') v = c - '0';
    else if (c >= 'a' && c <= 'f') v = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F') v = c - 'A' + 10;
    else return false;
    color = (color << 4) | v;
  }
  setLedColor(color);
  return true;
}
String getWebPassword() { return web_password; }
void setWebPassword(const String &pw) { web_password = pw; }
uint16_t getAutorunDelayMin() { return autorun_delay_min; }
uint16_t getAutorunDelayMax() { return autorun_delay_max; }
void setAutorunDelay(uint16_t minMs, uint16_t maxMs) {
  autorun_delay_min = minMs;
  autorun_delay_max = maxMs;
}

uint32_t pickAutorunDelayMs() {
  if (autorun_delay_max <= autorun_delay_min) return autorun_delay_min;
  return autorun_delay_min + (esp_random() % (autorun_delay_max - autorun_delay_min + 1));
}

uint8_t getWifiPower() { return wifi_power; }
void setWifiPower(uint8_t powerDb) {
  if (powerDb >= 8 && powerDb <= 84) wifi_power = powerDb;
}

String getWifiPowerLevel() {
  if (wifi_power <= 28) return "low";
  if (wifi_power <= 52) return "med";
  return "high";
}

void setWifiPowerLevel(const String &level) {
  String l = level;
  l.toLowerCase();
  l.trim();
  if (l == "low" || l == "min") wifi_power = 20; // 5 dBm
  else if (l == "med" || l == "medium" || l == "mid") wifi_power = 44; // 11 dBm
  else if (l == "high" || l == "max") wifi_power = 78; // 19.5 dBm
  else if (l.toInt() > 0) wifi_power = constrain(l.toInt(), 8, 84);
}

bool getWifiEnabled() { return wifi_enabled; }
void setWifiEnabled(bool on) { wifi_enabled = on; }

}
