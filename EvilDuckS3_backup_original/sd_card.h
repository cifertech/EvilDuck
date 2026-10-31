#pragma once

#include <Arduino.h>
#include <FS.h>

namespace sd_card {
  bool begin();
  bool ready();

  size_t totalBytes();
  size_t usedBytes();
  size_t freeBytes();

  bool exists(const String &path);
  bool mkdir(const String &path);
  bool remove(const String &path);
  bool rename(const String &from, const String &to);

  String readFile(const String &path);
  bool writeFile(const String &path, const String &content);
  bool appendFile(const String &path, const String &content);

  String listDir(const String &path = "/");
  String listScriptsJson();
  File openRead(const String &path);
}
