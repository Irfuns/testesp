#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <EEPROM.h>

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

struct WifiCfg {
  char ssid[32];
  char pass[32];
} cfg;

void saveCfg() {
  EEPROM.begin(128);
  EEPROM.put(0, cfg);
  EEPROM.commit();
  EEPROM.end();
}

bool loadCfg() {
  EEPROM.begin(128);
  EEPROM.get(0, cfg);
  EEPROM.end();
  return strlen(cfg.ssid) > 0;
}

void setup() {
  Serial.begin(115200);

  if (loadCfg()) {
    WiFi.begin(cfg.ssid, cfg.pass);
    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
      goto WIFI_OK;
    }
  }

  // fallback AP
  WiFi.softAP("ESP12F-SETUP");
  server.on("/", []() {
    server.send(200, "text/html",
      "<form action='/save'>SSID:<input name='s'><br>"
      "PASS:<input name='p'><br>"
      "<button>Save</button></form>");
  });

  server.on("/save", []() {
    strncpy(cfg.ssid, server.arg("s").c_str(), 31);
    strncpy(cfg.pass, server.arg("p").c_str(), 31);
    saveCfg();
    server.send(200, "text/plain", "Saved, reboot...");
    ESP.restart();
  });

  server.begin();
  return;

WIFI_OK:
  httpUpdater.setup(&server);
  server.begin();
}

void loop() {
  server.handleClient();
}
