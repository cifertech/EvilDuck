#include "webserver.h"
#include "debug.h"
#include "duckscript.h"
#include "settings.h"
#include "com.h"
#include "cli.h"
#include "config.h"
#include "webfiles.h"
#include "storage.h"
#include "script_log.h"
#include "script_library.h"
#include "attackmode.h"
#include "led.h"

#include <WiFi.h>
#include <SPIFFS.h>
#include <SD.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>

static AsyncWebServer server(80);
static AsyncWebSocket ws("/ws");
static File uploadFile;
static bool uploadRejected = false;
static bool fwUpdateOk = false;
static AsyncWebSocketClient *activeWsClient = nullptr;

static bool checkAuth(AsyncWebServerRequest *req) {
  String pw = settings::getWebPassword();
  if (!pw.length()) return true;
  if (req->hasHeader("X-Duck-Password")) {
    return req->header("X-Duck-Password") == pw;
  }
  if (req->hasParam("pw")) {
    return req->getParam("pw")->value() == pw;
  }
  return false;
}

static void sendAuthRequired(AsyncWebServerRequest *req) {
  req->send(401, "text/plain", "Unauthorized — set X-Duck-Password header or ?pw=");
}

static bool isScriptFile(const String &filename) {
  return filename.endsWith(".txt") || filename.endsWith(".duck");
}

static String scriptBaseName(const String &name) {
  if (name.startsWith("sd:") || name.startsWith("SD:")) return name.substring(3);
  return name;
}

static bool isSafeScriptName(const String &name) {
  String base = scriptBaseName(name);
  if (!base.length() || !isScriptFile(base)) return false;
  if (base.indexOf("..") >= 0 || base.indexOf('\\') >= 0) return false;
  for (size_t i = 0; i < base.length(); ++i) {
    char c = base[i];
    bool ok = (c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') ||
              c == '_' || c == '-' || c == '.' || c == '/';
    if (!ok) return false;
  }
  return true;
}

static bool parseScriptParam(const String &name, storage::Ref &ref) {
  if (!isSafeScriptName(name)) return false;
  if (!storage::parseRef(name, ref)) return false;
  return true;
}

static bool wantsSdStorage(AsyncWebServerRequest *req) {
  if (req->hasParam("storage")) {
    String v = req->getParam("storage")->value();
    v.toLowerCase();
    return v == "sd";
  }
  if (req->hasParam("storage", true)) {
    String v = req->getParam("storage", true)->value();
    v.toLowerCase();
    return v == "sd";
  }
  return false;
}

static String getContentType(const String &filename) {
  if (filename.endsWith(".html")) return "text/html";
  if (filename.endsWith(".css"))  return "text/css";
  if (filename.endsWith(".js"))   return "application/javascript";
  if (filename.endsWith(".png"))  return "image/png";
  if (filename.endsWith(".jpg"))  return "image/jpeg";
  if (filename.endsWith(".ico"))  return "image/x-icon";
  return "text/plain";
}

static bool loadScriptBody(const String &filename, String &body, String &err, int &httpCode);

static void handleListScripts(AsyncWebServerRequest *req) {
  if (!checkAuth(req)) { sendAuthRequired(req); return; }
  req->send(200, "application/json", storage::listAllScriptsJson());
}

static void handleLibrary(AsyncWebServerRequest *req) {
  if (!checkAuth(req)) { sendAuthRequired(req); return; }
  if (req->hasParam("s")) {
    String body, err;
    int code = 500;
    if (!loadScriptBody(req->getParam("s")->value(), body, err, code)) {
      req->send(code, "text/plain", err);
      return;
    }
    req->send(200, "text/plain", body);
    return;
  }
  if (req->hasParam("all")) {
    req->send(200, "application/json", script_library::listCombinedJson());
    return;
  }
  req->send(200, "application/json", script_library::listJson());
}

static void handleExecLog(AsyncWebServerRequest *req) {
  if (!checkAuth(req)) { sendAuthRequired(req); return; }
  if (req->hasParam("format") && req->getParam("format")->value() == "text") {
    req->send(200, "text/plain", script_log::getText());
    return;
  }
  req->send(200, "application/json", script_log::getJson());
}

static void handleStorageInfo(AsyncWebServerRequest *req) {
  req->send(200, "application/json", storage::infoJson());
}

static void handleDeleteScript(AsyncWebServerRequest *req) {
  if (!checkAuth(req)) { sendAuthRequired(req); return; }
  if (!req->hasParam("s")) {
    req->send(400, "text/plain", "Missing parameter s");
    return;
  }
  String filename = req->getParam("s")->value();
  storage::Ref ref;
  if (!parseScriptParam(filename, ref)) {
    req->send(400, "text/plain", "Invalid script name");
    return;
  }
  if (!storage::exists(ref)) {
    req->send(404, "text/plain", "Not found");
    return;
  }
  if (!storage::remove(ref)) {
    req->send(500, "text/plain", "Delete failed");
    return;
  }
  req->send(200, "text/plain", "Deleted");
}

static void handleRunScript(AsyncWebServerRequest *req) {
  if (!checkAuth(req)) { sendAuthRequired(req); return; }
  if (req->hasParam("cmd")) {
    String cmd = req->getParam("cmd")->value();
    if (!cmd.length()) {
      req->send(400, "text/plain", "Empty cmd");
      return;
    }
    if (!com::executeNow(cmd)) {
      req->send(400, "text/plain", "Command parse failed");
      return;
    }
    req->send(200, "text/plain", "Run: " + cmd);
    return;
  }

  if (!req->hasParam("s")) {
    req->send(400, "text/plain", "Missing parameter s");
    return;
  }
  String filename = req->getParam("s")->value();
  if (!isSafeScriptName(filename)) {
    req->send(400, "text/plain", "Invalid script name");
    return;
  }
  storage::Ref ref;
  if (!storage::resolveScript(filename, ref)) {
    String libraryBody;
    if (!script_library::readExample(filename, libraryBody)) {
      req->send(404, "text/plain", "Not found");
      return;
    }
    if (!storage::parseRef(filename, ref)) {
      req->send(400, "text/plain", "Invalid script name");
      return;
    }
    if (!storage::writeFile(ref, libraryBody)) {
      req->send(500, "text/plain", "Failed to install library script");
      return;
    }
    if (!storage::resolveScript(filename, ref)) {
      req->send(500, "text/plain", "Installed library script but could not resolve it");
      return;
    }
  }
  if (!duckscript::run(ref.display)) {
    req->send(409, "text/plain", "Already running or failed");
    return;
  }
  req->send(200, "text/plain", "Running");
}

static void handleSaveSettings(AsyncWebServerRequest *req) {
  if (!checkAuth(req)) { sendAuthRequired(req); return; }
  if (req->hasParam("ssid", true)) {
    String ssid = req->getParam("ssid", true)->value();
    if (!ssid.length() || ssid.length() > 32) {
      req->send(400, "text/plain", "Invalid ssid");
      return;
    }
    settings::setSSID(ssid);
  }

  if (req->hasParam("password", true)) {
    String password = req->getParam("password", true)->value();
    if (password.length() < 8 || password.length() > 63) {
      req->send(400, "text/plain", "Invalid password");
      return;
    }
    settings::setPassword(password);
  }

  if (req->hasParam("channel", true)) {
    uint8_t channel = static_cast<uint8_t>(req->getParam("channel", true)->value().toInt());
    if (channel < 1 || channel > 13) {
      req->send(400, "text/plain", "Invalid channel");
      return;
    }
    settings::setChannel(channel);
  }

  if (req->hasParam("autorun_script", true)) {
    String scriptName = req->getParam("autorun_script", true)->value();
    if (scriptName.length() && !isSafeScriptName(scriptName)) {
      req->send(400, "text/plain", "Invalid script name");
      return;
    }
    if (scriptName.length()) {
      storage::Ref ref;
      if (!storage::resolveScript(scriptName, ref)) {
        req->send(404, "text/plain", "Script not found");
        return;
      }
    }
    settings::setAutorun(scriptName);
  }

  if (req->hasParam("stealth", true)) {
    settings::setStealthMode(req->getParam("stealth", true)->value() == "1");
  }
  if (req->hasParam("hidden_ap", true)) {
    settings::setHiddenAp(req->getParam("hidden_ap", true)->value() == "1");
  }
  if (req->hasParam("disable_led", true)) {
    settings::setDisableLed(req->getParam("disable_led", true)->value() == "1");
  }
  if (req->hasParam("led_brightness", true)) {
    int b = req->getParam("led_brightness", true)->value().toInt();
    settings::setLedBrightness(constrain(b, 0, 255));
    led::setBrightness(settings::getLedBrightness());
  }
  if (req->hasParam("wifi_power", true)) {
    settings::setWifiPowerLevel(req->getParam("wifi_power", true)->value());
    WiFi.setTxPower((wifi_power_t)settings::getWifiPower());
  }
  if (req->hasParam("wifi_enabled", true)) {
    settings::setWifiEnabled(req->getParam("wifi_enabled", true)->value() == "1");
  }
  if (req->hasParam("cpu_freq", true)) {
    int cf = req->getParam("cpu_freq", true)->value().toInt();
    settings::setCpuFreq(cf);
    setCpuFrequencyMhz(settings::getCpuFreq());
  }
  if (req->hasParam("led_color", true)) {
    if (!settings::setLedColorHex(req->getParam("led_color", true)->value())) {
      req->send(400, "text/plain", "Invalid LED color");
      return;
    }
  }
  if (req->hasParam("web_password", true)) {
    settings::setWebPassword(req->getParam("web_password", true)->value());
  }
  if (req->hasParam("autorun_delay_min", true) && req->hasParam("autorun_delay_max", true)) {
    settings::setAutorunDelay(
        static_cast<uint16_t>(req->getParam("autorun_delay_min", true)->value().toInt()),
        static_cast<uint16_t>(req->getParam("autorun_delay_max", true)->value().toInt()));
  }

  settings::save();
  if (settings::getDisableLed()) {
    led::setColor(0, 0, 0);
  } else if (!duckscript::running()) {
    uint32_t color = settings::getLedColor();
    led::setColor((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
  }
  req->send(200, "text/plain", "OK (reboot to apply hidden AP)");
}

static void handleGetSettings(AsyncWebServerRequest *req) {
  if (!checkAuth(req)) { sendAuthRequired(req); return; }
  String json = "{";
  json += "\"ssid\":\"" + settings::getSSID() + "\",";
  json += "\"password\":\"" + settings::getPassword() + "\",";
  json += "\"channel\":" + String(settings::getChannel()) + ",";
  json += "\"autorun_script\":\"" + settings::getAutorun() + "\",";
  json += "\"stealth\":" + String(settings::getStealthMode() ? "true" : "false") + ",";
  json += "\"hidden_ap\":" + String(settings::getHiddenAp() ? "true" : "false") + ",";
  json += "\"disable_led\":" + String(settings::getDisableLed() ? "true" : "false") + ",";
  json += "\"led_brightness\":" + String(settings::getLedBrightness()) + ",";
  json += "\"wifi_power\":\"" + settings::getWifiPowerLevel() + "\",";
  json += "\"wifi_enabled\":" + String(settings::getWifiEnabled() ? "true" : "false") + ",";
  json += "\"cpu_freq\":" + String(settings::getCpuFreq()) + ",";
  json += "\"led_color\":\"" + settings::getLedColorHex() + "\",";
  json += "\"web_password\":\"" + settings::getWebPassword() + "\",";
  json += "\"autorun_delay_min\":" + String(settings::getAutorunDelayMin()) + ",";
  json += "\"autorun_delay_max\":" + String(settings::getAutorunDelayMax());
  json += "}";
  req->send(200, "application/json", json);
}

static void onUpload(AsyncWebServerRequest *request, String filename, size_t index,
                     uint8_t *data, size_t len, bool final) {
  if (!index) {
    if (!checkAuth(request)) {
      uploadRejected = true;
      request->send(401, "text/plain", "Unauthorized");
      return;
    }
    uploadRejected = false;
    if (!isSafeScriptName(filename)) {
      uploadRejected = true;
      request->send(400, "text/plain", "Invalid script name");
      return;
    }
    storage::Ref ref;
    if (wantsSdStorage(request)) {
      if (!storage::parseRef("sd:" + scriptBaseName(filename), ref)) {
        uploadRejected = true;
        request->send(400, "text/plain", "SD not available");
        return;
      }
#if SD_ENABLED
      uploadFile = SD.open(ref.path, FILE_WRITE);
#else
      uploadRejected = true;
      request->send(400, "text/plain", "SD disabled");
      return;
#endif
    } else {
      uploadFile = SPIFFS.open("/" + scriptBaseName(filename), FILE_WRITE);
    }
    if (!uploadFile) {
      uploadRejected = true;
      request->send(500, "text/plain", "Open failed");
      return;
    }
  }

  if (!uploadRejected && uploadFile) {
    uploadFile.write(data, len);
  }

  if (final) {
    if (uploadFile) uploadFile.close();
    if (!uploadRejected) {
      request->send(200, "text/plain", "Uploaded");
    }
  }
}

static void onFirmwareUpload(AsyncWebServerRequest *request, String filename, size_t index,
                             uint8_t *data, size_t len, bool final) {
  (void)filename;
  if (!index) {
    fwUpdateOk = false;
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      request->send(500, "text/plain", "Update begin failed");
      return;
    }
  }

  if (Update.write(data, len) != len) {
    Update.abort();
    request->send(500, "text/plain", "Update write failed");
    return;
  }

  if (final) {
    fwUpdateOk = Update.end(true);
  }
}

static void handleStatus(AsyncWebServerRequest *req) {
  if (!checkAuth(req)) { sendAuthRequired(req); return; }
  String json = "{";
  json += "\"running\":";
  json += duckscript::running() ? "true" : "false";
  json += ",\"autorunEnabled\":";
  json += settings::autorunEnabled() ? "true" : "false";
  json += ",\"autorunScript\":\"";
  json += settings::getAutorun();
  json += "\",\"ssid\":\"";
  json += settings::getSSID();
  json += "\",\"channel\":";
  json += String(settings::getChannel());
  json += ",\"sdReady\":";
  json += storage::sdReady() ? "true" : "false";
  json += ",\"script\":\"";
  json += duckscript::runningScript();
  json += "\",\"line\":";
  json += String(duckscript::currentLine());
  json += ",\"lastError\":\"";
  json += duckscript::lastError();
  json += "\",\"hid\":";
  json += attackmode::hidEnabled() ? "true" : "false";
  json += ",\"storage\":";
  json += attackmode::storageEnabled() ? "true" : "false";
  json += "}";
  req->send(200, "application/json", json);
}

static void handleSetLed(AsyncWebServerRequest *req) {
  if (!checkAuth(req)) { sendAuthRequired(req); return; }
  if (req->hasParam("brightness", true)) {
    int b = req->getParam("brightness", true)->value().toInt();
    settings::setLedBrightness(constrain(b, 0, 255));
    led::setBrightness(settings::getLedBrightness());
  }
  if (req->hasParam("color", true)) {
    settings::setLedColorHex(req->getParam("color", true)->value());
  }
  settings::save();
  if (settings::getDisableLed()) {
    led::setColor(0, 0, 0);
  } else if (!duckscript::running()) {
    uint32_t color = settings::getLedColor();
    led::setColor((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
  }
  req->send(200, "text/plain", "OK");
}

static bool loadScriptBody(const String &filename, String &body, String &err, int &httpCode) {
  String name = filename;
  name.trim();
  if (!name.length()) {
    err = "Empty name";
    httpCode = 400;
    return false;
  }
  if (!isSafeScriptName(name)) {
    err = "Invalid script name";
    httpCode = 400;
    return false;
  }
  storage::Ref ref;
  if (storage::resolveScriptForLoad(name, ref)) {
    body = storage::readFile(ref);
    httpCode = 200;
    return true;
  }
  if (script_library::readFirmware(name, body)) {
    httpCode = 200;
    return true;
  }
  err = "Not found: " + name;
  httpCode = 404;
  return false;
}

static void handleGetScript(AsyncWebServerRequest *req) {
  if (!checkAuth(req)) { sendAuthRequired(req); return; }
  if (!req->hasParam("s")) {
    req->send(400, "text/plain", "Missing parameter s");
    return;
  }
  String body, err;
  int code = 500;
  if (!loadScriptBody(req->getParam("s")->value(), body, err, code)) {
    req->send(code, "text/plain", err);
    return;
  }
  req->send(200, "text/plain", body);
}

static void handleLoadScriptPost(AsyncWebServerRequest *req) {
  if (!checkAuth(req)) { sendAuthRequired(req); return; }
  if (!req->hasParam("name", true)) {
    req->send(400, "text/plain", "Missing name");
    return;
  }
  String body, err;
  int code = 500;
  if (!loadScriptBody(req->getParam("name", true)->value(), body, err, code)) {
    req->send(code, "text/plain", err);
    return;
  }
  req->send(200, "text/plain", body);
}

static void handleSaveScript(AsyncWebServerRequest *req) {
  if (!checkAuth(req)) { sendAuthRequired(req); return; }
  if (!req->hasParam("name", true) || !req->hasParam("content", true)) {
    req->send(400, "text/plain", "Missing name or content");
    return;
  }
  String filename = req->getParam("name", true)->value();
  filename.trim();
  if (!isSafeScriptName(filename)) {
    req->send(400, "text/plain", "Invalid script name");
    return;
  }
  storage::Ref ref;
  if (wantsSdStorage(req)) {
    String sdName = filename;
    if (!sdName.startsWith("sd:") && !sdName.startsWith("SD:")) {
      sdName = "sd:" + scriptBaseName(filename);
    }
    if (!parseScriptParam(sdName, ref)) {
      req->send(400, "text/plain", "SD not available");
      return;
    }
  } else {
    if (!parseScriptParam(filename, ref)) {
      req->send(400, "text/plain", "Invalid name");
      return;
    }
  }
  String content = req->getParam("content", true)->value();
  if (!storage::writeFile(ref, content)) {
    req->send(500, "text/plain", "Write failed");
    return;
  }
  req->send(200, "text/plain", "Saved to " + ref.display);
}

static void handleSendLine(AsyncWebServerRequest *req) {
  if (!checkAuth(req)) { sendAuthRequired(req); return; }
  if (!req->hasParam("line", true)) {
    req->send(400, "text/plain", "Missing line");
    return;
  }
  String line = req->getParam("line", true)->value();
  if (!line.length()) {
    req->send(400, "text/plain", "Empty line");
    return;
  }
  if (!com::executeNow(line)) {
    req->send(400, "text/plain", "Command parse failed");
    return;
  }
  req->send(200, "text/plain", "OK");
}

static void wsOut(const char *s) {
  if (activeWsClient) activeWsClient->text(s);
}

static void onWsEvent(AsyncWebSocket *serverPtr, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
  (void)serverPtr;
  if (type != WS_EVT_DATA) return;
  AwsFrameInfo *info = reinterpret_cast<AwsFrameInfo *>(arg);
  if (!info || info->opcode != WS_TEXT || !info->final || info->index != 0) return;
  String cmd;
  cmd.reserve(len + 1);
  for (size_t i = 0; i < len; ++i) cmd += static_cast<char>(data[i]);
  activeWsClient = client;
  cli::parse(cmd.c_str(), wsOut, true);
  activeWsClient = nullptr;
}

static void handleStatic(AsyncWebServerRequest *req) {
  String path = req->url();
  if (path == "/" || path == "/index.html" || path == "/index.htm") {
    req->send_P(200, "text/html", INDEX_HTML);
    return;
  }
  if (path == "/settings.html") {
    req->send_P(200, "text/html", SETTINGS_HTML);
    return;
  }
  if (path == "/terminal.html") {
    req->send_P(200, "text/html", TERMINAL_HTML);
    return;
  }
  if (path == "/credits.html") {
    req->send_P(200, "text/html", CREDITS_HTML);
    return;
  }
  if (path == "/error404.html") {
    req->send_P(404, "text/html", ERROR404_HTML);
    return;
  }
  if (path == "/style.css") {
    req->send_P(200, "text/css", STYLE_CSS);
    return;
  }
  if (path == "/index.js") {
    req->send_P(200, "application/javascript", INDEX_JS);
    return;
  }
  if (path == "/script.js") {
    req->send_P(200, "application/javascript", INDEX_JS);
    return;
  }
  if (path == "/settings.js") {
    req->send_P(200, "application/javascript", SETTINGS_JS);
    return;
  }
  if (path == "/terminal.js") {
    req->send_P(200, "application/javascript", TERMINAL_JS);
    return;
  }
  if (SPIFFS.exists(path)) {
    req->send(SPIFFS, path, getContentType(path));
  } else {
    req->send_P(404, "text/html", ERROR404_HTML);
  }
}

namespace webserver {

void begin() {
  WiFi.mode(WIFI_AP);
  WiFi.setSleep(true); // Power management: modem sleep
  WiFi.setTxPower((wifi_power_t)settings::getWifiPower());
  int hidden = settings::getHiddenAp() ? 1 : 0;
  WiFi.softAP(settings::getSSID().c_str(),
              settings::getPassword().c_str(),
              settings::getChannel(),
              hidden);

  debug::info("AP SSID: " + settings::getSSID());
  debug::info("AP IP: " + WiFi.softAPIP().toString());

  server.on("/list", HTTP_GET, handleListScripts);
  server.on("/library", HTTP_GET, handleLibrary);
  server.on("/exec/log", HTTP_GET, handleExecLog);
  server.on("/storage", HTTP_GET, handleStorageInfo);
  server.on("/script", HTTP_GET, handleGetScript);
  server.on("/script/load", HTTP_POST, handleLoadScriptPost);
  server.on("/script", HTTP_POST, handleSaveScript);
  server.on("/delete", HTTP_GET, handleDeleteScript);
  server.on("/run", HTTP_GET, handleRunScript);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/led", HTTP_POST, handleSetLed);
  server.on("/settings", HTTP_GET, handleGetSettings);
  server.on("/settings", HTTP_POST, handleSaveSettings);
  server.on("/send", HTTP_POST, handleSendLine);
  server.on("/upload", HTTP_POST,
            [](AsyncWebServerRequest *request) { /* handled in onUpload */ },
            onUpload);
  server.on("/update", HTTP_POST,
            [](AsyncWebServerRequest *request) {
              if (fwUpdateOk) {
                request->send(200, "text/plain", "OK");
                delay(100);
                ESP.restart();
              } else {
                request->send(500, "text/plain", "FAIL");
              }
            },
            onFirmwareUpload);

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.onNotFound(handleStatic);

  server.begin();
  debug::info("Webserver started");
}

}
