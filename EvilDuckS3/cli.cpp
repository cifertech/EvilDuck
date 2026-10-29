#include "cli.h"
#include "duckscript.h"
#include "settings.h"
#include "com.h"
#include "debug.h"
#include "duck_spiffs.h"
#include "storage.h"
#include "sd_card.h"
#include "attackmode.h"
#include "script_log.h"
#include "led.h"
#include "webserver.h"
#include <WiFi.h>

namespace {
String lineBuffer;
PrintFunction activePrint = nullptr;

String trimLine(const String &in) {
  String s = in;
  s.trim();
  return s;
}

bool startsWithNoCase(const String &s, const char *pfx) {
  String a = s;
  String b = String(pfx);
  a.toLowerCase();
  b.toLowerCase();
  return a.startsWith(b);
}

bool equalsNoCase(const String &aIn, const char *bIn) {
  String a = aIn;
  String b = String(bIn);
  a.toLowerCase();
  b.toLowerCase();
  return a == b;
}

String normalizePath(const String &in) {
  if (!in.length()) return "/";
  if (in[0] == '/') return in;
  return "/" + in;
}

void out(const String &msg) {
  if (activePrint) activePrint(msg.c_str());
}

void serialOut(const char *s) { Serial.println(s); }

void printHelp() {
  out("CLI commands:");
  out("  help, ram, version, settings, set, reset, status, log");
  out("  run <file.txt>, stop, send <duckscript-line>");
  out("  autorun <file.txt|off>");
  out("  mem, format, ls <path>, cat <file>");
  out("  create <file>, remove <file>, rename <a> <b>");
  out("  write <file> <content>");
  out("  stream <file>, read, close");
  out("  sd, sdls, import <sd:name>, export <name>");
  out("  Paths: payload.txt (SPIFFS) or sd:payload.txt (SD card)");
  out("Ducky extras: VAR, IF/ELSE, WHILE, SAVE/APPEND/READ, ATTACKMODE, LOCALE");
  out("Stealth settings: stealth, hidden_ap, disable_led, web_password,");
  out("  autorun_delay_min, autorun_delay_max (use: set <name> <value>)");
}

void executeCommand(const String &raw) {
  String cmd = trimLine(raw);
  if (!cmd.length()) return;

  if (startsWithNoCase(cmd, "help")) {
    printHelp();
    return;
  }
  if (startsWithNoCase(cmd, "ram")) {
    out(String(ESP.getFreeHeap()) + " bytes available");
    return;
  }
  if (startsWithNoCase(cmd, "version")) {
    out("Version S3-port");
    return;
  }
  if (startsWithNoCase(cmd, "settings")) {
    String s = "";
    s += "ssid=" + settings::getSSID() + "\n";
    s += "password=" + settings::getPassword() + "\n";
    s += "channel=" + String(settings::getChannel()) + "\n";
    s += "autorun=" + settings::getAutorun() + "\n";
    s += "stealth=" + String(settings::getStealthMode() ? 1 : 0) + "\n";
    s += "hidden_ap=" + String(settings::getHiddenAp() ? 1 : 0) + "\n";
    s += "disable_led=" + String(settings::getDisableLed() ? 1 : 0) + "\n";
    s += "web_password=" + settings::getWebPassword() + "\n";
    s += "autorun_delay_min=" + String(settings::getAutorunDelayMin()) + "\n";
    s += "autorun_delay_max=" + String(settings::getAutorunDelayMax()) + "\n";
    s += "brightness=" + String(settings::getLedBrightness()) + "\n";
    s += "wifi_power=" + settings::getWifiPowerLevel() + " (" + String(settings::getWifiPower()) + ")\n";
    s += "wifi_enabled=" + String(settings::getWifiEnabled() ? 1 : 0) + "\n";
    s += "cpu_freq=" + String(settings::getCpuFreq()) + "MHz\n";
    out(s);
    return;
  }
  if (startsWithNoCase(cmd, "log")) {
    out(script_log::getText());
    return;
  }
  if (startsWithNoCase(cmd, "set ")) {
    String rest = trimLine(cmd.substring(4));
    int sp = rest.indexOf(' ');
    if (sp < 0) {
      out("Usage: set <name> <value>");
      return;
    }
    String name = trimLine(rest.substring(0, sp));
    String value = trimLine(rest.substring(sp + 1));
    if (value.startsWith("\"") && value.endsWith("\"") && value.length() >= 2) {
      value = value.substring(1, value.length() - 1);
    }
    name.toLowerCase();
    if (name == "ssid") settings::setSSID(value);
    else if (name == "password") settings::setPassword(value);
    else if (name == "channel") settings::setChannel(static_cast<uint8_t>(value.toInt()));
    else if (name == "autorun") settings::setAutorun(value);
    else if (name == "stealth") settings::setStealthMode(value == "1" || equalsNoCase(value, "true"));
    else if (name == "hidden_ap") settings::setHiddenAp(value == "1" || equalsNoCase(value, "true"));
    else if (name == "disable_led") settings::setDisableLed(value == "1" || equalsNoCase(value, "true"));
    else if (name == "web_password") settings::setWebPassword(value);
    else if (name == "brightness" || name == "led_brightness") {
      int b = value.toInt();
      settings::setLedBrightness(constrain(b, 0, 255));
      led::setBrightness(settings::getLedBrightness());
    } else if (name == "wifi_power" || name == "tx_power") {
      settings::setWifiPowerLevel(value);
      WiFi.setTxPower((wifi_power_t)settings::getWifiPower());
    } else if (name == "wifi") {
      bool en = (value == "1" || equalsNoCase(value, "on") || equalsNoCase(value, "true"));
      settings::setWifiEnabled(en);
      if (!en) {
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_OFF);
        out("Wi-Fi turned OFF (cool / silent mode)");
      } else {
        webserver::begin();
        out("Wi-Fi turned ON");
      }
    } else if (name == "cpu_freq") {
      int f = value.toInt();
      settings::setCpuFreq(f);
      setCpuFrequencyMhz(settings::getCpuFreq());
    }
    else if (name == "autorun_delay_min") {
      uint16_t maxv = settings::getAutorunDelayMax();
      settings::setAutorunDelay(static_cast<uint16_t>(value.toInt()), maxv);
    } else if (name == "autorun_delay_max") {
      uint16_t minv = settings::getAutorunDelayMin();
      settings::setAutorunDelay(minv, static_cast<uint16_t>(value.toInt()));
    } else {
      out("Unknown setting");
      return;
    }
    settings::save();
    out("> set \"" + name + "\" to \"" + value + "\"");
    return;
  }
  if (startsWithNoCase(cmd, "reset")) {
    settings::reset();
    out("Settings reset");
    return;
  }
  if (startsWithNoCase(cmd, "status")) {
    out(String("running=") + (duckscript::running() ? "1" : "0") +
        ", script=" + duckscript::runningScript() +
        ", line=" + String(duckscript::currentLine()) +
        ", autorun=" + (settings::autorunEnabled() ? settings::getAutorun() : String("off")) +
        ", sd=" + (storage::sdReady() ? "1" : "0") +
        ", hid=" + (attackmode::hidEnabled() ? "1" : "0") +
        ", storage_usb=" + (attackmode::storageEnabled() ? "1" : "0"));
    if (duckscript::lastError().length()) out("last_error=" + duckscript::lastError());
    return;
  }
  if (equalsNoCase(cmd, "sd") || startsWithNoCase(cmd, "sd status")) {
    out(storage::infoJson());
    return;
  }
  if (startsWithNoCase(cmd, "sdls")) {
    if (!storage::sdReady()) {
      out("SD not ready");
      return;
    }
    out(sd_card::listDir("/scripts"));
    return;
  }
  if (startsWithNoCase(cmd, "import ")) {
    String name = trimLine(cmd.substring(7));
    storage::Ref src;
    if (!storage::parseRef(name.startsWith("sd:") ? name : "sd:" + name, src)) {
      out("Invalid name");
      return;
    }
    if (!storage::exists(src)) {
      out("SD file not found");
      return;
    }
    String base = name;
    if (base.startsWith("sd:")) base = base.substring(3);
    if (storage::copyToSpiffs(src, base)) out("Imported to SPIFFS: " + base);
    else out("Import failed");
    return;
  }
  if (startsWithNoCase(cmd, "export ")) {
    String name = trimLine(cmd.substring(7));
    storage::Ref dst;
    if (!storage::parseRef("sd:" + name, dst)) {
      out("Invalid name");
      return;
    }
    if (storage::copyToSd(name, dst)) out("Exported to " + dst.display);
    else out("Export failed");
    return;
  }
  if (startsWithNoCase(cmd, "run ")) {
    String name = trimLine(cmd.substring(4));
    if (!duckscript::run(name)) out("Busy or failed to start");
    else out("> started \"" + name + "\"");
    return;
  }
  if (startsWithNoCase(cmd, "stop")) {
    duckscript::stop();
    out("Stopped");
    return;
  }
  if (startsWithNoCase(cmd, "send ")) {
    String line = cmd.substring(5);
    if (!com::executeNow(line)) out("Send failed");
    else out("OK");
    return;
  }
  if (startsWithNoCase(cmd, "autorun ")) {
    String value = trimLine(cmd.substring(8));
    if (startsWithNoCase(value, "off")) settings::setAutorun("");
    else settings::setAutorun(value);
    settings::save();
    out("Autorun saved");
    return;
  }

  if (startsWithNoCase(cmd, "mem")) {
    out(String(spiffs::size()) + " byte");
    out(String(spiffs::usedBytes()) + " byte used");
    out(String(spiffs::freeBytes()) + " byte free");
    return;
  }
  if (startsWithNoCase(cmd, "format")) {
    out(spiffs::format() ? "Formatted SPIFFS" : "Format failed");
    return;
  }
  if (startsWithNoCase(cmd, "ls")) {
    String path = "/";
    if (cmd.length() > 2) path = trimLine(cmd.substring(2));
    out(spiffs::listDir(path));
    return;
  }
  if (startsWithNoCase(cmd, "cat ")) {
    String path = trimLine(cmd.substring(4));
    storage::Ref ref;
    if (storage::parseRef(path, ref) && storage::exists(ref)) {
      out(storage::readFile(ref));
    } else {
      out(spiffs::readFile(path));
    }
    return;
  }
  if (startsWithNoCase(cmd, "create ")) {
    String path = trimLine(cmd.substring(7));
    out(spiffs::create(path) ? ("> created file \"" + normalizePath(path) + "\"") : "Create failed");
    return;
  }
  if (startsWithNoCase(cmd, "remove ")) {
    String path = trimLine(cmd.substring(7));
    out(spiffs::remove(path) ? ("> removed file \"" + normalizePath(path) + "\"") : "Remove failed");
    return;
  }
  if (startsWithNoCase(cmd, "rename ")) {
    String rest = trimLine(cmd.substring(7));
    int sp = rest.indexOf(' ');
    if (sp < 0) {
      out("Usage: rename <fileA> <fileB>");
      return;
    }
    String a = trimLine(rest.substring(0, sp));
    String b = trimLine(rest.substring(sp + 1));
    out(spiffs::rename(a, b) ? ("> renamed \"" + normalizePath(a) + "\" to \"" + normalizePath(b) + "\"") : "Rename failed");
    return;
  }
  if (startsWithNoCase(cmd, "write ")) {
    String rest = trimLine(cmd.substring(6));
    int sp = rest.indexOf(' ');
    if (sp < 0) {
      out("Usage: write <file> <content>");
      return;
    }
    String file = trimLine(rest.substring(0, sp));
    String content = rest.substring(sp + 1);
    out(spiffs::append(file, content) ? ("> wrote to file \"" + normalizePath(file) + "\"") : "Write failed");
    return;
  }
  if (startsWithNoCase(cmd, "stream ")) {
    String file = trimLine(cmd.substring(7));
    out(spiffs::streamOpen(file) ? ("> opened stream \"" + normalizePath(file) + "\"") : "Stream open failed");
    return;
  }
  if (startsWithNoCase(cmd, "close")) {
    spiffs::streamClose();
    out("> closed stream");
    return;
  }
  if (startsWithNoCase(cmd, "read")) {
    if (!spiffs::streaming()) {
      out("No stream");
      return;
    }
    if (!spiffs::streamAvailable()) {
      out("> END");
      return;
    }
    char buf[1024];
    spiffs::streamRead(buf, sizeof(buf));
    out(String(buf));
    return;
  }
  if (spiffs::streaming()) {
    spiffs::streamWrite((cmd + "\n").c_str(), cmd.length() + 1);
    out("> Written data to file");
    return;
  }

  out("Unknown command, type 'help'");
}
}

namespace cli {
void begin() {
  lineBuffer.reserve(256);
  debug::info("CLI ready: type 'help'");
}

void parse(const char *input, PrintFunction printfunc, bool echo) {
  activePrint = printfunc;
  if (echo) out(String("# ") + input);
  executeCommand(String(input));
}

void update() {
  while (Serial.available() > 0) {
    char c = static_cast<char>(Serial.read());
    if (c == '\r') continue;
    if (c == '\n') {
      parse(lineBuffer.c_str(), serialOut, false);
      lineBuffer = "";
    } else if (lineBuffer.length() < 512) {
      lineBuffer += c;
    }
  }
}
}
