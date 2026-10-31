#include "usb_msc.h"
#include "config.h"
#include "debug.h"

#if SD_ENABLED && defined(CONFIG_TINYUSB_MSC_ENABLED) && CONFIG_TINYUSB_MSC_ENABLED

#include "USB.h"
#include "USBMSC.h"
#include "SD.h"

static USBMSC MSC;
static bool mscActive = false;
static File diskFile;
static const char *DISK_IMG = "/sys/usb_disk.img";
static const uint16_t SECTOR_SIZE = 512;
static uint32_t sectorCount = 0;

static bool ensureDiskImage() {
  if (!SD.exists("/sys")) SD.mkdir("/sys");
  if (!SD.exists(DISK_IMG)) {
    File f = SD.open(DISK_IMG, FILE_WRITE);
    if (!f) return false;
    uint8_t zero[512] = {};
    for (uint32_t i = 0; i < 8192; ++i) f.write(zero, 512);
    f.close();
  }
  return true;
}

static void openDiskFile() {
  if (!diskFile) diskFile = SD.open(DISK_IMG, FILE_WRITE);
}

static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) {
  (void)offset;
  openDiskFile();
  if (!diskFile) return -1;
  uint32_t pos = lba * SECTOR_SIZE;
  diskFile.seek(pos);
  return diskFile.read(static_cast<uint8_t *>(buffer), bufsize) == bufsize ? (int32_t)bufsize : -1;
}

static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize) {
  (void)offset;
  openDiskFile();
  if (!diskFile) return -1;
  uint32_t pos = lba * SECTOR_SIZE;
  diskFile.seek(pos);
  return diskFile.write(buffer, bufsize) == bufsize ? (int32_t)bufsize : -1;
}

static bool onStartStop(uint8_t power_condition, bool start, bool load_eject) {
  (void)power_condition;
  (void)load_eject;
  if (start) {
    openDiskFile();
  } else if (diskFile) {
    diskFile.close();
  }
  return true;
}

namespace usb_msc {

bool begin() {
  if (mscActive) return true;
  if (!SD.cardType()) {
    debug::error("MSC: no SD card");
    return false;
  }
  if (!ensureDiskImage()) return false;
  sectorCount = 8192;
  MSC.vendorID("Evil");
  MSC.productID("Duck");
  MSC.productRevision("1.0");
  MSC.mediaPresent(true);
  MSC.onRead(onRead);
  MSC.onWrite(onWrite);
  MSC.onStartStop(onStartStop);
  if (!MSC.begin(sectorCount, SECTOR_SIZE)) {
    debug::error("MSC begin failed");
    return false;
  }
  mscActive = true;
  debug::info("USB MSC active (4MB on SD)");
  return true;
}

void end() {
  if (!mscActive) return;
  MSC.end();
  if (diskFile) diskFile.close();
  mscActive = false;
}

bool active() { return mscActive; }

}

#else

namespace usb_msc {

bool begin() {
#if SD_ENABLED
  debug::warn("USB MSC disabled — enable TinyUSB MSC in board menu");
#else
  debug::warn("USB MSC not available (SD disabled)");
#endif
  return false;
}

void end() {}
bool active() { return false; }

}

#endif
