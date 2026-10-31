#include "storage.h"
#include "duck_spiffs.h"
#include "sd_card.h"
#include "config.h"
#include <SPIFFS.h>

namespace storage {

namespace {

bool isScriptFile(const String &name) {
  return name.endsWith(".txt") || name.endsWith(".duck");
}

String basenameOnly(const String &name) {
  String n = name;
  if (n.startsWith("sd:") || n.startsWith("SD:")) n = n.substring(3);
  if (n.startsWith("/sd/")) n = n.substring(4);
  if (n.startsWith("sd/")) n = n.substring(3);
  while (n.length() && n[0] == '/') n = n.substring(1);
  return n;
}

String spiffsPath(const String &name) {
  String n = basenameOnly(name);
  if (!n.length()) return "";
  return n[0] == '/' ? n : String("/") + n;
}

bool spiffsExistsPath(const String &path) {
  if (!path.length()) return false;
  if (SPIFFS.exists(path)) return true;
  if (path.startsWith("/")) return SPIFFS.exists(path.substring(1));
  return SPIFFS.exists(String("/") + path);
}

String dirPrefix(const String &dirPath) {
  String d = dirPath;
  if (d.startsWith("/")) d = d.substring(1);
  while (d.length() && d.endsWith("/")) d.remove(d.length() - 1);
  return d;
}

String scriptRelPath(const String &dirPath, const String &entryName) {
  String rel = entryName;
  if (rel.startsWith("/")) rel = rel.substring(1);
  if (isScriptFile(rel) && rel.indexOf('/') < 0) {
    String prefix = dirPrefix(dirPath);
    if (prefix.length()) rel = prefix + "/" + rel;
  }
  return rel;
}

bool ensureSpiffsParents(const String &filePath) {
  if (!filePath.length() || filePath == "/") return true;
  int slash = filePath.lastIndexOf('/');
  if (slash <= 0) return true;
  String dir = filePath.substring(0, slash);
  if (SPIFFS.exists(dir)) return true;
  if (!ensureSpiffsParents(dir)) return false;
  return SPIFFS.mkdir(dir);
}

String sdPath(const String &name) {
  String n = basenameOnly(name);
  if (!n.length()) return "";
  return String("/scripts/") + n;
}

}

bool begin() {
  bool ok = spiffs::begin();
#if SD_ENABLED
  sd_card::begin();
#endif
  return ok;
}

bool sdReady() {
#if SD_ENABLED
  return sd_card::ready();
#else
  return false;
#endif
}

bool parseRef(const String &input, Ref &out) {
  String s = input;
  s.trim();
  if (!s.length()) return false;
  if (!isScriptFile(s) && s.indexOf('.') < 0) return false;

  if (s.startsWith("sd:") || s.startsWith("SD:")) {
    out.volume = Volume::SD;
    out.path = sdPath(s);
    out.display = "sd:" + basenameOnly(s);
    return out.path.length() > 0;
  }
  if (s.startsWith("/sd/") || s.startsWith("sd/")) {
    out.volume = Volume::SD;
    out.path = sdPath(s);
    out.display = "sd:" + basenameOnly(s);
    return out.path.length() > 0;
  }

  out.volume = Volume::SPIFFS;
  out.path = spiffsPath(s);
  out.display = basenameOnly(s);
  return out.path.length() > 0 && isScriptFile(s);
}

String toDisplay(const Ref &ref) {
  return ref.display;
}

bool exists(const Ref &ref) {
  if (ref.volume == Volume::SD) {
#if SD_ENABLED
    return sd_card::ready() && sd_card::exists(ref.path);
#else
    return false;
#endif
  }
  return spiffsExistsPath(ref.path);
}

bool resolveScriptForLoad(const String &name, Ref &out) {
  if (parseRef(name, out) && exists(out)) return true;
  if (name.indexOf(':') >= 0) return false;
  if (name.indexOf('/') >= 0) {
    String alt = name;
    if (alt.startsWith("/")) alt = alt.substring(1);
    if (parseRef(alt, out) && exists(out)) return true;
    return false;
  }
  static const char *DIRS[] = {
      "demos/", "windows/", "linux/", "macos/", "advanced/",
      "examples/", "scripts/", nullptr};
  for (int i = 0; DIRS[i]; ++i) {
    String tryName = String(DIRS[i]) + name;
    if (parseRef(tryName, out) && exists(out)) return true;
  }
  return false;
}

bool remove(const Ref &ref) {
  if (ref.volume == Volume::SD) {
#if SD_ENABLED
    return sd_card::ready() && sd_card::remove(ref.path);
#else
    return false;
#endif
  }
  return spiffs::remove(ref.path);
}

String readFile(const Ref &ref) {
  if (ref.volume == Volume::SD) {
#if SD_ENABLED
    return sd_card::readFile(ref.path);
#else
    return "";
#endif
  }
  if (spiffsExistsPath(ref.path)) return spiffs::readFile(ref.path);
  String alt = ref.path.startsWith("/") ? ref.path.substring(1) : String("/") + ref.path;
  if (spiffsExistsPath(alt)) return spiffs::readFile(alt);
  return "";
}

bool writeFile(const Ref &ref, const String &content) {
  if (ref.volume == Volume::SD) {
#if SD_ENABLED
    if (!sd_card::ready()) return false;
    return sd_card::writeFile(ref.path, content);
#else
    return false;
#endif
  }
  if (!ensureSpiffsParents(ref.path)) return false;
  File f = SPIFFS.open(ref.path, FILE_WRITE);
  if (!f) {
    String alt = ref.path.startsWith("/") ? ref.path.substring(1) : String("/") + ref.path;
    if (!ensureSpiffsParents(alt)) return false;
    f = SPIFFS.open(alt, FILE_WRITE);
  }
  if (!f) return false;
  size_t n = f.print(content);
  f.close();
  return n == content.length();
}

bool appendFile(const Ref &ref, const String &content) {
  if (ref.volume == Volume::SD) {
#if SD_ENABLED
    if (!sd_card::ready()) return false;
    return sd_card::appendFile(ref.path, content);
#else
    return false;
#endif
  }
  return spiffs::append(ref.path, content);
}

File openRead(const Ref &ref) {
  if (ref.volume == Volume::SD) {
#if SD_ENABLED
    if (!sd_card::ready()) return File();
    return sd_card::openRead(ref.path);
#else
    return File();
#endif
  }
  return SPIFFS.open(ref.path, FILE_READ);
}

bool resolveScript(const String &name, Ref &out) {
  if (!parseRef(name, out)) return false;
  if (exists(out)) return true;

  if (out.volume == Volume::SPIFFS) {
    Ref alt;
    alt.volume = Volume::SD;
    alt.path = sdPath(name);
    alt.display = "sd:" + basenameOnly(name);
    if (sdReady() && exists(alt)) {
      out = alt;
      return true;
    }
  } else {
    Ref alt;
    alt.volume = Volume::SPIFFS;
    alt.path = spiffsPath(name);
    alt.display = basenameOnly(name);
    if (exists(alt)) {
      out = alt;
      return true;
    }
  }
  return false;
}

String listSpiffsScriptsJson() {
  String json = "[";
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  bool first = true;
  while (file) {
    if (!file.isDirectory()) {
      String filename = String(file.name());
      if (filename.startsWith("/")) filename = filename.substring(1);
      if (isScriptFile(filename)) {
        if (!first) json += ",";
        first = false;
        json += "\"" + filename + "\"";
      }
    }
    file = root.openNextFile();
  }
  json += "]";
  return json;
}

void appendSpiffsDir(const String &dirPath, String &json, bool &first) {
  File dir = SPIFFS.open(dirPath);
  if (!dir || !dir.isDirectory()) return;
  File entry = dir.openNextFile();
  while (entry) {
    String path = String(entry.name());
    String rel = scriptRelPath(dirPath, path);

    if (isScriptFile(rel)) {
      if (!first) json += ",";
      first = false;
      json += "\"" + rel + "\"";
    } else if (entry.isDirectory()) {
      appendSpiffsDir(path, json, first);
    } else {
      File probe = SPIFFS.open(path);
      if (probe && probe.isDirectory()) {
        probe.close();
        appendSpiffsDir(path, json, first);
      } else if (probe) {
        probe.close();
      }
    }
    entry = dir.openNextFile();
  }
}

String listAllScriptsJson() {
  String json = "[";
  bool first = true;
  File root = SPIFFS.open("/");
  if (root && root.isDirectory()) {
    File f = root.openNextFile();
    while (f) {
      String rel = String(f.name());
      if (rel.startsWith("/")) rel = rel.substring(1);
      if (!f.isDirectory() && isScriptFile(rel) && rel.indexOf('/') < 0) {
        if (!first) json += ",";
        first = false;
        json += "\"" + rel + "\"";
      }
      f = root.openNextFile();
    }
  }
  static const char *DIRS[] = {
      "/examples", "/demos", "/windows", "/linux",
      "/macos", "/advanced", "/scripts", nullptr};
  for (int i = 0; DIRS[i]; ++i) {
    appendSpiffsDir(DIRS[i], json, first);
  }

#if SD_ENABLED
  if (sd_card::ready()) {
    String sdList = sd_card::listScriptsJson();
    if (sdList.length() > 2) {
      String inner = sdList.substring(1, sdList.length() - 1);
      if (inner.length()) {
        if (!first) json += ",";
        json += inner;
      }
    }
  }
#endif

  json += "]";
  return json;
}

String infoJson() {
  String j = "{";
  j += "\"spiffs\":{\"total\":" + String(spiffs::size());
  j += ",\"used\":" + String(spiffs::usedBytes());
  j += ",\"free\":" + String(spiffs::freeBytes()) + "}";
  j += ",\"sd\":{\"ready\":";
  j += sdReady() ? "true" : "false";
#if SD_ENABLED
  if (sdReady()) {
    j += ",\"total\":" + String(sd_card::totalBytes());
    j += ",\"used\":" + String(sd_card::usedBytes());
    j += ",\"free\":" + String(sd_card::freeBytes());
  }
#endif
  j += "}}";
  return j;
}

bool copyToSpiffs(const Ref &fromSd, const String &spiffsName) {
#if !SD_ENABLED
  (void)fromSd;
  (void)spiffsName;
  return false;
#else
  if (fromSd.volume != Volume::SD || !sd_card::ready()) return false;
  String content = sd_card::readFile(fromSd.path);
  Ref dest;
  dest.volume = Volume::SPIFFS;
  dest.path = spiffsPath(spiffsName);
  dest.display = basenameOnly(spiffsName);
  return writeFile(dest, content);
#endif
}

bool copyToSd(const String &spiffsName, const Ref &toSd) {
#if !SD_ENABLED
  (void)spiffsName;
  (void)toSd;
  return false;
#else
  if (toSd.volume != Volume::SD || !sd_card::ready()) return false;
  String content = spiffs::readFile(spiffsPath(spiffsName));
  return sd_card::writeFile(toSd.path, content);
#endif
}

}
