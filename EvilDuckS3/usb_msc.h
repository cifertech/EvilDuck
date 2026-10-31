#pragma once

namespace usb_msc {
  void init();
  bool begin();
  void end();
  bool active();
}
