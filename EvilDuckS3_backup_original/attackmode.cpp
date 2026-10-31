#include "attackmode.h"
#include "usb_msc.h"
#include "script_log.h"
#include "debug.h"

static bool hidOn = true;
static bool storageOn = false;

namespace attackmode {

void begin() {
  hidOn = true;
  storageOn = false;
}

bool hidEnabled() { return hidOn; }
bool storageEnabled() { return storageOn; }

void setHid(bool on) { hidOn = on; }

void setStorage(bool on) {
  if (on == storageOn) return;
  storageOn = on;
  if (storageOn) {
    if (usb_msc::begin()) script_log::info("USB storage enabled");
    else script_log::warn("USB storage failed (check SD)");
  } else {
    usb_msc::end();
    script_log::info("USB storage disabled");
  }
}

void setOff() {
  setStorage(false);
  hidOn = false;
}

void setHidOnly() {
  setStorage(false);
  hidOn = true;
}

void setStorageOnly() {
  hidOn = false;
  setStorage(true);
}

void setHidStorage() {
  hidOn = true;
  setStorage(true);
}

bool applyFromString(const String &modes) {
  String m = modes;
  m.toUpperCase();
  if (m.indexOf("OFF") >= 0) { setOff(); return true; }
  bool wantHid = m.indexOf("HID") >= 0 || m.indexOf("KEYBOARD") >= 0;
  bool wantSt = m.indexOf("STORAGE") >= 0 || m.indexOf("MSC") >= 0 || m.indexOf("DISK") >= 0;
  if (!wantHid && !wantSt) wantHid = true;
  hidOn = wantHid;
  setStorage(wantSt);
  return true;
}

}
