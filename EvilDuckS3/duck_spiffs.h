#pragma once
#include <Arduino.h>

namespace spiffs {
  bool begin();
  size_t size();
  size_t usedBytes();
  size_t freeBytes();
  String listDir(const String &path = "/");
  bool create(const String &path);
  bool remove(const String &path);
  bool rename(const String &from, const String &to);
  bool append(const String &path, const String &content);
  String readFile(const String &path);
  bool format();

  bool streamOpen(const String &path);
  void streamClose();
  bool streaming();
  size_t streamWrite(const char *data, size_t len);
  size_t streamRead(char *buffer, size_t maxLen);
  bool streamAvailable();
}
