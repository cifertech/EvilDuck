#pragma once

#include <Arduino.h>
#include <FS.h>

namespace storage {

enum class Volume { SPIFFS, SD };

struct Ref {
  Volume volume = Volume::SPIFFS;
  String path;       // e.g. /payload.txt
  String display;    // e.g. payload.txt or sd:payload.txt
};

bool begin();
bool sdReady();

bool parseRef(const String &input, Ref &out);
String toDisplay(const Ref &ref);

bool exists(const Ref &ref);
bool remove(const Ref &ref);
String readFile(const Ref &ref);
bool writeFile(const Ref &ref, const String &content);
bool appendFile(const Ref &ref, const String &content);

File openRead(const Ref &ref);
bool resolveScript(const String &name, Ref &out);
bool resolveScriptForLoad(const String &name, Ref &out);

String listSpiffsScriptsJson();
String listAllScriptsJson();

String infoJson();
bool copyToSpiffs(const Ref &fromSd, const String &spiffsName);
bool copyToSd(const String &spiffsName, const Ref &toSd);

}
