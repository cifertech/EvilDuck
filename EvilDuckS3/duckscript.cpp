#include "duckscript.h"
#include "com.h"
#include "debug.h"
#include "config.h"
#include "storage.h"
#include "script_log.h"
#include "duck_vars.h"
#include "led.h"
#include "settings.h"

static File scriptFile;
static storage::Ref scriptRef;
static bool active = false;
static uint32_t lineNumber = 0;
static String lastErr;

static bool skipBlock = false;
static bool ifBranchTaken = false;
static uint32_t whileStartPos = 0;
static uint32_t whileStartLine = 0;
static String whileKey;
static bool inWhile = false;
static bool pendingRestart = false;

static void loadNextLine();
static void comDone();
static void comRepeat();
static void comError();

namespace {

String trimLine(const String &s) {
  String t = s;
  t.trim();
  return t;
}

bool startsWithNoCase(const String &s, const char *pfx) {
  String a = s; a.toLowerCase();
  String b = String(pfx); b.toLowerCase();
  return a.startsWith(b);
}

void setStatusColor(int r, int g, int b) {
  if (!settings::getDisableLed()) led::setColor(r, g, b);
}

void setIdleColor() {
  if (settings::getDisableLed()) {
    led::setColor(0, 0, 0);
    return;
  }
  uint32_t color = settings::getLedColor();
  led::setColor((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
}

bool evalCondition(const String &expr) {
  String e = duck_vars::expand(trimLine(expr));
  e.trim();
  int eq = e.indexOf("==");
  if (eq > 0) {
    String left = e.substring(0, eq); left.trim();
    String right = e.substring(eq + 2); right.trim();
    return left == right;
  }
  if (e.startsWith("$")) return duck_vars::truthy(e);
  return e.length() > 0 && e != "0" && e != "false";
}

bool isControlLine(const String &line, bool &execute) {
  execute = false;
  String l = trimLine(line);
  if (!l.length()) return true;

  if (startsWithNoCase(l, "ENDIF") || startsWithNoCase(l, "END IF")) {
    skipBlock = false;
    ifBranchTaken = false;
    return true;
  }
  if (startsWithNoCase(l, "ENDWHILE") || startsWithNoCase(l, "END WHILE")) {
    if (inWhile && scriptFile && whileKey.length()) {
      if (duck_vars::truthy(whileKey)) {
        scriptFile.seek(whileStartPos);
        lineNumber = whileStartLine;
        execute = false;
        return true;
      }
    }
    inWhile = false;
    whileKey = "";
    return true;
  }
  if (startsWithNoCase(l, "ELSE")) {
    skipBlock = ifBranchTaken;
    return true;
  }
  if (startsWithNoCase(l, "IF ")) {
    bool cond = evalCondition(l.substring(3));
    ifBranchTaken = cond;
    skipBlock = !cond;
    return true;
  }
  if (startsWithNoCase(l, "WHILE ")) {
    whileKey = trimLine(l.substring(6));
    whileStartPos = scriptFile.position();
    whileStartLine = lineNumber;
    inWhile = true;
    if (!duck_vars::truthy(whileKey)) skipBlock = true;
    else skipBlock = false;
    return true;
  }

  if (skipBlock) return true;
  execute = true;
  return false;
}

}

namespace duckscript {

bool run(const String &filename) {
  if (active) return false;

  if (!storage::resolveScript(filename, scriptRef)) {
    lastErr = "Cannot open script: " + filename;
    debug::error(lastErr);
    script_log::error(lastErr);
    setStatusColor(50, 0, 0);
    return false;
  }

  scriptFile = storage::openRead(scriptRef);
  if (!scriptFile) {
    lastErr = "Open failed: " + scriptRef.display;
    script_log::error(lastErr);
    setStatusColor(50, 0, 0);
    return false;
  }

  active = true;
  lineNumber = 0;
  skipBlock = false;
  inWhile = false;
  pendingRestart = false;
  duck_vars::clear();
  script_log::clear();
  script_log::info("Started " + scriptRef.display);
  setStatusColor(35, 0, 45);

  com::onDone(comDone);
  com::onRepeat(comRepeat);
  com::onError(comError);

  loadNextLine();
  return true;
}

void stop() {
  active = false;
  if (scriptFile) scriptFile.close();
  script_log::info("Stopped");
  setIdleColor();
  if (pendingRestart) {
    pendingRestart = false;
    debug::info("Restarting ESP32 for USB mode change after script completion...");
    delay(500);
    ESP.restart();
  }
}

void update() {}

bool running() { return active; }
String runningScript() { return active ? scriptRef.display : String(); }
uint32_t currentLine() { return lineNumber; }
String lastError() { return lastErr; }

void scheduleRestart() {
  pendingRestart = true;
}

bool hasPendingRestart() {
  return pendingRestart;
}

}

static void loadNextLine() {
  if (!active) return;

  bool queuedCommand = false;
  while (scriptFile.available()) {
    String line = scriptFile.readStringUntil('\n');
    line.replace("\r", "");
    ++lineNumber;
    String trimmed = line;
    trimmed.trim();
    if (!trimmed.length() || trimmed.startsWith("REM") || trimmed.startsWith("rem") || trimmed.startsWith("//")) {
      continue;
    }
    script_log::setLine(lineNumber, scriptRef.display);

    bool execute = false;
    if (isControlLine(line, execute)) {
      if (execute) {
        if (!com::executeNow(line)) {
          lastErr = "Error at line " + String(lineNumber);
          script_log::error(lastErr);
          duckscript::stop();
          setStatusColor(50, 0, 0);
          return;
        }
        break;
      }
      continue;
    }

    com::send(line);
    queuedCommand = true;
    break;
  }

  if (!active) return;
  if (!queuedCommand && !scriptFile.available()) {
    script_log::info("Finished");
    duckscript::stop();
    return;
  }
}

static void comDone() {
  loadNextLine();
}

static void comRepeat() {}

static void comError() {
  duckscript::stop();
  setStatusColor(50, 0, 0);
}
