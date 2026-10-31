#include "debug.h"
#include "config.h"

namespace debug {

void begin() {
  Serial.begin(DEBUG_SERIAL_BAUD);
  delay(200);
}

void info(const String &msg) {
  Serial.println("[I] " + msg);
}

void error(const String &msg) {
  Serial.println("[E] " + msg);
}

}
