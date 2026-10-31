#include "com.h"
#include "debug.h"
#include "hid_backend.h"
#include "keyboard_locale.h"
#include "duck_keys.h"
#include "led.h"
#include "storage.h"
#include "duck_vars.h"
#include "attackmode.h"
#include "script_log.h"
#include "settings.h"
#include <cstdlib>

static com::cb_t cbDone = nullptr;
static com::cb_t cbRepeat = nullptr;
static com::cb_t cbError = nullptr;

static bool pending = false;
static String pendingLine;
static String lastExecutableLine;
static uint32_t defaultDelayMs = 0;

namespace {

String trimLine(const String &s) {
  int start = 0;
  while (start < (int)s.length() && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '\n')) start++;
  int end = s.length() - 1;
  while (end >= start && (s[end] == ' ' || s[end] == '\t' || s[end] == '\r' || s[end] == '\n')) end--;
  if (end < start) return "";
  return s.substring(start, end + 1);
}

bool startsWithNoCase(const String &s, const char *pfx) {
  String a = s; a.toLowerCase();
  String b = String(pfx); b.toLowerCase();
  return a.startsWith(b);
}

bool equalsNoCase(const String &aIn, const char *bIn) {
  String a = aIn; String b = String(bIn);
  a.toLowerCase(); b.toLowerCase();
  return a == b;
}

bool pressNamedKey(const String &line);

uint32_t parseDelayMs(const String &s) {
  int sp = s.indexOf(' ');
  if (sp < 0) return 0;
  return (uint32_t)s.substring(sp + 1).toInt();
}

uint8_t parseNumberToken(const String &token) {
  String t = trimLine(token);
  t.toLowerCase();
  if (t.startsWith("0x")) return static_cast<uint8_t>(strtoul(t.c_str(), nullptr, 16));
  return static_cast<uint8_t>(t.toInt());
}

bool parseFilePayload(const String &line, const char *cmd, String &fileRef, String &payload) {
  String rest = trimLine(line.substring(strlen(cmd)));
  int sp = rest.indexOf(" STRING ");
  if (sp < 0) sp = rest.indexOf(" string ");
  if (sp < 0) return false;
  fileRef = trimLine(rest.substring(0, sp));
  payload = rest.substring(sp + 8);
  payload = duck_vars::expand(payload);
  if (payload.startsWith("\"") && payload.endsWith("\"") && payload.length() >= 2) {
    payload = payload.substring(1, payload.length() - 1);
  }
  return fileRef.length() > 0;
}

bool fileIoWrite(const String &fileRef, const String &payload, bool append) {
  storage::Ref ref;
  if (!storage::parseRef(fileRef, ref)) return false;
  bool ok = append ? storage::appendFile(ref, payload) : storage::writeFile(ref, payload);
  if (ok) script_log::info(String(append ? "APPEND " : "SAVE ") + ref.display);
  return ok;
}

bool execKeycodeLine(const String &rest) {
  String r = trimLine(rest);
  uint8_t modifiers = 0;
  uint8_t keys[6] = {};
  uint8_t keyCount = 0;
  bool gotMod = false;

  while (r.length()) {
    int sp = r.indexOf(' ');
    String tok = (sp < 0) ? trimLine(r) : trimLine(r.substring(0, sp));
    if (sp < 0) r = "";
    else r = r.substring(sp + 1);
    if (!tok.length()) continue;

    uint8_t val = parseNumberToken(tok);
    if (!gotMod) {
      modifiers = val;
      gotMod = true;
    } else if (keyCount < 6) {
      keys[keyCount++] = val;
    }
  }

  if (!gotMod && !keyCount) return false;
  if (!attackmode::hidEnabled()) return true;
  hid_backend::pressReport(modifiers, keys, keyCount);
  hid_backend::releaseAll();
  return true;
}

bool execLine(const String &raw, bool rememberLine = true) {
  String line = trimLine(raw);
  if (!line.length()) return true;
  if (startsWithNoCase(line, "REM") || line.startsWith("//")) return true;

  if (startsWithNoCase(line, "DELAY ") || startsWithNoCase(line, "WAIT ")) {
    delay(parseDelayMs(line));
    return true;
  }
  if (startsWithNoCase(line, "DEFAULT_DELAY ") || startsWithNoCase(line, "DEFAULTDELAY ")) {
    defaultDelayMs = parseDelayMs(line);
    return true;
  }
  if (startsWithNoCase(line, "REPEAT ") || startsWithNoCase(line, "REPLAY ")) {
    uint32_t n = parseDelayMs(line);
    if (!lastExecutableLine.length()) return false;
    for (uint32_t i = 0; i < n; ++i) {
      if (!execLine(lastExecutableLine, false)) return false;
      if (defaultDelayMs > 0) delay(defaultDelayMs);
    }
    return true;
  }
  if (startsWithNoCase(line, "LOCALE ")) {
    return keyboard_locale::setLocaleByName(trimLine(line.substring(7)));
  }
  if (startsWithNoCase(line, "LED ")) {
    if (settings::getDisableLed()) return true;
    String rest = trimLine(line.substring(4));
    if (equalsNoCase(rest, "OFF")) {
      led::setColor(0, 0, 0);
      return true;
    }
    int sp1 = rest.indexOf(' ');
    int sp2 = rest.indexOf(' ', sp1 + 1);
    int r = sp1 > 0 ? rest.substring(0, sp1).toInt() : 0;
    int g = (sp1 > 0 && sp2 > sp1) ? rest.substring(sp1 + 1, sp2).toInt() : 0;
    int b = (sp2 > sp1) ? rest.substring(sp2 + 1).toInt() : 0;
    led::setColor(r, g, b);
    return true;
  }
  if (startsWithNoCase(line, "VAR ") || startsWithNoCase(line, "DEFINE ")) {
    int cmdLen = startsWithNoCase(line, "VAR ") ? 4 : 7;
    String rest = trimLine(line.substring(cmdLen));
    int eq = rest.indexOf('=');
    if (eq < 0) return false;
    String name = trimLine(rest.substring(0, eq));
    String val = duck_vars::expand(trimLine(rest.substring(eq + 1)));
    if (val.startsWith("\"") && val.endsWith("\"") && val.length() >= 2) {
      val = val.substring(1, val.length() - 1);
    }
    return duck_vars::set(name, val);
  }
  if (startsWithNoCase(line, "READ ")) {
    String rest = trimLine(line.substring(5));
    int sp = rest.lastIndexOf(' ');
    if (sp < 0) return false;
    String fileRef = trimLine(rest.substring(0, sp));
    String varName = trimLine(rest.substring(sp + 1));
    storage::Ref ref;
    if (!storage::parseRef(fileRef, ref)) return false;
    String data = storage::readFile(ref);
    return duck_vars::set(varName, data);
  }
  if (startsWithNoCase(line, "SAVE ")) {
    String fileRef, payload;
    if (!parseFilePayload(line, "SAVE", fileRef, payload)) return false;
    return fileIoWrite(fileRef, payload, false);
  }
  if (startsWithNoCase(line, "APPEND ")) {
    String fileRef, payload;
    if (!parseFilePayload(line, "APPEND", fileRef, payload)) return false;
    return fileIoWrite(fileRef, payload, true);
  }
  if (startsWithNoCase(line, "ATTACKMODE ")) {
    return attackmode::applyFromString(trimLine(line.substring(11)));
  }
  if (startsWithNoCase(line, "KEYCODE ")) {
    if (!execKeycodeLine(line.substring(8))) return false;
    if (rememberLine) lastExecutableLine = line;
    if (defaultDelayMs > 0) delay(defaultDelayMs);
    return true;
  }
  if (startsWithNoCase(line, "STRINGLN ")) {
    if (!attackmode::hidEnabled()) return true;
    keyboard_locale::write(duck_vars::expand(line.substring(9)));
    hid_backend::pressEnter();
    if (rememberLine) lastExecutableLine = line;
    if (defaultDelayMs > 0) delay(defaultDelayMs);
    return true;
  }
  if (startsWithNoCase(line, "STRING ")) {
    if (!attackmode::hidEnabled()) return true;
    keyboard_locale::write(duck_vars::expand(line.substring(7)));
    if (rememberLine) lastExecutableLine = line;
    if (defaultDelayMs > 0) delay(defaultDelayMs);
    return true;
  }

  if (!attackmode::hidEnabled()) return true;

  if (pressNamedKey(line)) {
    if (rememberLine) lastExecutableLine = line;
    if (defaultDelayMs > 0) delay(defaultDelayMs);
    return true;
  }

  duck_keys::pressLineTokens(line);
  if (rememberLine) lastExecutableLine = line;
  if (defaultDelayMs > 0) delay(defaultDelayMs);
  return true;
}

bool pressNamedKey(const String &line) {
  uint8_t code = 0;
  if (!hid_backend::keyNameToHid(line, code)) return false;
  hid_backend::pressKeycode(code, 0);
  return true;
}

}

namespace com {

void onDone(cb_t cb) { cbDone = cb; }
void onRepeat(cb_t cb) { cbRepeat = cb; }
void onError(cb_t cb) { cbError = cb; }

void send(const String &line) {
  pendingLine = line;
  pending = true;
}

bool executeNow(const String &line) {
  return execLine(line);
}

void update() {
  if (!pending) return;
  pending = false;
  if (!execLine(pendingLine)) {
    if (cbError) cbError();
    return;
  }
  if (cbDone) cbDone();
}

}
