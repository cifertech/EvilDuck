#include "duck_spiffs.h"
#include "debug.h"
#include <SPIFFS.h>

namespace {
File streamFile;
String streamPath;
size_t streamReadOffset = 0;

String normalizePath(const String &in) {
  if (!in.length()) return "/";
  if (in[0] == '/') return in;
  return "/" + in;
}
}

namespace spiffs {

bool begin() {
  if (!SPIFFS.begin(true)) {
    debug::error("SPIFFS mount failed");
    return false;
  }
  debug::info("SPIFFS mounted");
  return true;
}

size_t size() { return SPIFFS.totalBytes(); }
size_t usedBytes() { return SPIFFS.usedBytes(); }
size_t freeBytes() { return SPIFFS.totalBytes() - SPIFFS.usedBytes(); }

String listDir(const String &path) {
  String output;
  File root = SPIFFS.open(normalizePath(path));
  if (!root || !root.isDirectory()) return "";

  File file = root.openNextFile();
  while (file) {
    output += String(file.name());
    if (!file.isDirectory()) {
      output += " (" + String(file.size()) + ")\n";
    } else {
      output += "/\n";
    }
    file = root.openNextFile();
  }
  return output;
}

bool create(const String &path) {
  File f = SPIFFS.open(normalizePath(path), FILE_WRITE);
  if (!f) return false;
  f.close();
  return true;
}

bool remove(const String &path) {
  return SPIFFS.remove(normalizePath(path));
}

bool rename(const String &from, const String &to) {
  return SPIFFS.rename(normalizePath(from), normalizePath(to));
}

bool append(const String &path, const String &content) {
  File f = SPIFFS.open(normalizePath(path), FILE_APPEND);
  if (!f) return false;
  size_t written = f.print(content);
  f.close();
  return written == content.length();
}

String readFile(const String &path) {
  File f = SPIFFS.open(normalizePath(path), FILE_READ);
  if (!f) return "";
  String out = f.readString();
  f.close();
  return out;
}

bool format() { return SPIFFS.format(); }

bool streamOpen(const String &path) {
  streamClose();
  streamPath = normalizePath(path);
  streamReadOffset = 0;
  streamFile = SPIFFS.open(streamPath, FILE_APPEND);
  return static_cast<bool>(streamFile);
}

void streamClose() {
  if (streamFile) streamFile.close();
  streamPath = "";
  streamReadOffset = 0;
}

bool streaming() {
  return static_cast<bool>(streamFile);
}

size_t streamWrite(const char *data, size_t len) {
  if (!streamFile) return 0;
  return streamFile.write(reinterpret_cast<const uint8_t *>(data), len);
}

size_t streamRead(char *buffer, size_t maxLen) {
  if (!streamFile || maxLen == 0 || !streamPath.length()) return 0;
  File rf = SPIFFS.open(streamPath, FILE_READ);
  if (!rf) return 0;
  rf.seek(streamReadOffset);
  size_t readLen = rf.readBytes(buffer, maxLen - 1);
  buffer[readLen] = '\0';
  streamReadOffset += readLen;
  rf.close();
  return readLen;
}

bool streamAvailable() {
  if (!streamFile || !streamPath.length()) return false;
  File rf = SPIFFS.open(streamPath, FILE_READ);
  if (!rf) return false;
  bool avail = rf.size() > streamReadOffset;
  rf.close();
  return avail;
}

}
