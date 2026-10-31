#pragma once
#include <Arduino.h>

typedef void (*PrintFunction)(const char *s);

namespace cli {
  void begin();
  void update();
  void parse(const char *input, PrintFunction printfunc, bool echo = true);
}
