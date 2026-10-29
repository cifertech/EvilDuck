#include "script_library.h"
#include "storage.h"
#include "duck_spiffs.h"
#include <SPIFFS.h>

struct ExampleScript {
  const char *name;
  const char *body;
};

static const ExampleScript EXAMPLES[] = {
  {"examples/hello.txt",
   "REM EvilDuck example — types Hello World in notepad (Windows)\n"
   "DEFAULTDELAY 100\n"
   "GUI r\n"
   "DELAY 500\n"
   "STRING notepad\n"
   "ENTER\n"
   "DELAY 500\n"
   "STRING Hello from EvilDuck!\n"},
  {"examples/linux_shell.txt",
   "REM Open terminal on many Linux desktops\n"
   "DEFAULTDELAY 100\n"
   "GUI\n"
   "DELAY 300\n"
   "STRING terminal\n"
   "ENTER\n"
   "DELAY 500\n"
   "STRING echo EvilDuck was here\n"
   "ENTER\n"},
  {"examples/save_log.txt",
   "REM Device file I/O example (writes to SD)\n"
   "VAR $msg = captured\n"
   "SAVE sd:loot.txt STRING $msg\n"
   "APPEND sd:loot.txt STRING \\nmore data\n"},
  {"examples/attackmode_demo.txt",
   "REM Toggle USB modes (HID + storage)\n"
   "ATTACKMODE HID STORAGE\n"
   "DELAY 1000\n"
   "STRING storage and keyboard active\n"},
  {"windows/hacker_terminals_3_monitors.txt",
   "REM Windows - open three hacker terminal graphics on each monitor\n"
   "DEFAULTDELAY 50\n"
   "GUI r\n"
   "DELAY 450\n"
   "STRING cmd /k C:\\Users\\CiferTech\\Desktop\\EvilDuckS3\\scripts\\windows\\run_hacker_terminals.cmd\n"
   "ENTER\n"},
};

namespace script_library {

bool allExamplesOnDisk() {
  for (size_t i = 0; i < sizeof(EXAMPLES) / sizeof(EXAMPLES[0]); ++i) {
    storage::Ref ref;
    if (!storage::parseRef(String(EXAMPLES[i].name), ref)) return false;
    if (!storage::exists(ref)) return false;
  }
  return true;
}

void installIfNeeded() {
  if (allExamplesOnDisk()) return;
  for (size_t i = 0; i < sizeof(EXAMPLES) / sizeof(EXAMPLES[0]); ++i) {
    storage::Ref ref;
    if (!storage::parseRef(String(EXAMPLES[i].name), ref)) continue;
    storage::writeFile(ref, String(EXAMPLES[i].body));
  }
  File f = SPIFFS.open("/examples/.installed", FILE_WRITE);
  if (f) { f.print("1"); f.close(); }
}

String listJson() {
  String j = "[";
  bool first = true;
  for (size_t i = 0; i < sizeof(EXAMPLES) / sizeof(EXAMPLES[0]); ++i) {
    if (!first) j += ",";
    first = false;
    j += "\"" + String(EXAMPLES[i].name) + "\"";
  }
  j += "]";
  return j;
}

String listSpiffsOnlyJson() {
  return storage::listAllScriptsJson();
}

String listCombinedJson() {
  String j = listJson();
  String spiffs = storage::listAllScriptsJson();
  if (spiffs.length() <= 2) return j;
  String inner = spiffs.substring(1, spiffs.length() - 1);
  if (!inner.length()) return j;
  if (j.length() <= 2) return spiffs;
  j.remove(j.length() - 1);
  j += ",";
  j += inner;
  j += "]";
  return j;
}

bool readFirmware(const String &name, String &out) {
  for (size_t i = 0; i < sizeof(EXAMPLES) / sizeof(EXAMPLES[0]); ++i) {
    if (name == EXAMPLES[i].name) {
      out = String(EXAMPLES[i].body);
      return true;
    }
  }
  return false;
}

bool readExample(const String &name, String &out) {
  if (readFirmware(name, out)) return true;
  storage::Ref ref;
  if (storage::parseRef(name, ref) && storage::exists(ref)) {
    out = storage::readFile(ref);
    return true;
  }
  return false;
}

}
