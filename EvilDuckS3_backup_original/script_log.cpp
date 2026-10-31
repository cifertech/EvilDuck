#include "script_log.h"

static const size_t MAX_LINES = 48;
static String lines[MAX_LINES];
static size_t head = 0;
static size_t count = 0;
static uint32_t currentLine = 0;
static String currentScript;

namespace script_log {

static void push(const String &level, const String &msg) {
  String row = "[" + level + "] ";
  if (currentScript.length()) row += currentScript + ":" + String(currentLine) + " ";
  row += msg;
  lines[head] = row;
  head = (head + 1) % MAX_LINES;
  if (count < MAX_LINES) ++count;
}

void clear() {
  head = count = 0;
  currentLine = 0;
  currentScript = "";
  for (size_t i = 0; i < MAX_LINES; ++i) lines[i] = "";
}

void info(const String &msg) { push("INFO", msg); }
void warn(const String &msg) { push("WARN", msg); }
void error(const String &msg) { push("ERR", msg); }

void setLine(uint32_t line, const String &scriptName) {
  currentLine = line;
  if (scriptName.length()) currentScript = scriptName;
}

String getText() {
  String out;
  if (!count) return out;
  size_t start = (head + MAX_LINES - count) % MAX_LINES;
  for (size_t i = 0; i < count; ++i) {
    out += lines[(start + i) % MAX_LINES] + "\n";
  }
  return out;
}

String getJson() {
  String j = "{\"script\":\"" + currentScript + "\",\"line\":" + String(currentLine) + ",\"log\":[";
  if (!count) {
    j += "]}";
    return j;
  }
  size_t start = (head + MAX_LINES - count) % MAX_LINES;
  for (size_t i = 0; i < count; ++i) {
    if (i) j += ",";
    String s = lines[(start + i) % MAX_LINES];
    s.replace("\\", "\\\\");
    s.replace("\"", "\\\"");
    j += "\"" + s + "\"";
  }
  j += "]}";
  return j;
}

}
