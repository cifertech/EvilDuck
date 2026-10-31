#pragma once
#include <Arduino.h>

namespace com {
  typedef void (*cb_t)();

  void onDone(cb_t cb);
  void onRepeat(cb_t cb);
  void onError(cb_t cb);

  void send(const String &line);
  bool executeNow(const String &line);
  void update();
}
