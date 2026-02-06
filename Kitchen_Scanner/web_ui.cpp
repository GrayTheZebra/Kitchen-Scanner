#include "web_ui.h"
#include <ESP8266WiFi.h>
#include <LittleFS.h>

#include "topics.h"
#include "app_config.h"
#include "wifi_mgr.h"
#include "mqtt_mgr.h"
#include "tof_mgr.h"
#include "scanner_mgr.h"

ESP8266WebServer server(80);

String htmlEscape(const String& s) {
  String o;
  o.reserve(s.length() + 8);
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    switch (c) {
      case '&': o += F("&amp;"); break;
      case '<': o += F("&lt;"); break;
      case '>': o += F("&gt;"); break;
      case '"': o += F("&quot;"); break;
      case '\'': o += F("&#39;"); break;
      default: o += c; break;
    }
  }
  return o;
}

static String pageHeader(const String& title) {
  String h;
  h += F("<!doctype html><html><head><meta charset='utf-8'>");
  h += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
  h += F("<title>");
  h += htmlEscape(title);
  h += F("</title>");
  h += F("<style>body{font-family:sans-serif;max-width:900px;margin:20px auto;padding:0 12px}");
  h += F("input,select{width:100%;padding:10px;margin:6px 0}button{padding:10px 14px;margin:6px 0}");
  h += F(".card{border:1px solid #ddd;border-radius:10px;padding:14px;margin:12px 0}");
  h += F("code{background:#f5f5f5;padding:2px 6px;border-radius:6px}</style>");
  h += F("</head><body>");
  h += F("<h2>");
  h += htmlEscape(title);
  h += F("</h2>");
  h += F("<p><a href='/'>Status</a> | <a href='/wifi'>WLAN</a> | <a href='/mqtt'>MQTT</a> | ");
  h += F("<a href='/reboot'>Reboot</a> | <a href='/factory' onclick='return confirm(\"Wirklich Factory Reset?\")'>Factory</a></p>");
  return h;
}
static String pageFooter() { return F("</body></html>"); }

static void handleRoot() {
  String p = pageHeader("Kitchen Scanner - Status");

  p += F("<div class='card'><b>Modus:</b> ");
  p += wifi_ap_mode ? F("AP (Setup)") : F("STA (WLAN)");
  p += F("<br><b>IP:</b> ");
  p += wifi_ap_mode ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
  p += F("<br><b>SSID:</b> ");
  p += htmlEscape(wifi_ap_mode ? String(AP_SSID) : cfg.wifi_ssid);
  p += F("</div>");

  p += F("<div class='card'><b>Scan-Modus:</b> ");
  p += htmlEscape(String(modeToText()));
  p += F("<br><b>Topic IN:</b> <code>scanner/in</code>");
  p += F("<br><b>Topic OUT:</b> <code>scanner/out</code>");
  p += F("<br><b>Topic Display:</b> <code>scanner/display</code>");
  p += F("<br><b>Topic Status:</b> <code>scanner/status</code>");

  p += F("<br><b>MQTT:</b> ");
  if (cfg.mqtt_host.length() == 0) p += F("nicht konfiguriert");
  else {
    p += htmlEscape(cfg.mqtt_host);
    p += ":";
    p += String(cfg.mqtt_port);
    p += mqtt.connected() ? F(" (verbunden)") : F(" (nicht verbunden)");
  }

  p += F("<br><b>ToF:</b> ");
  p += tof_ok ? F("OK (VL53L0X)") : F("nicht erkannt");
  p += F("<br><b>Trigger:</b> ");
  p += trig_holding ? F("HOLD ON") : F("HOLD OFF");
  p += F("<br><b>Re-Arm:</b> ");
  p += wait_for_far ? F("warte auf FAR") : F("bereit");
  p += F("</div>");

  p += pageFooter();
  server.send(200, "text/html; charset=utf-8", p);
}

// --- WLAN/MQTT Seiten + Actions: wie bei dir, nur ausgelagert ---
static void handleWifiPage() {
  String p = pageHeader("WLAN Einstellungen");
  p += F("<div class='card'><form method='POST' action='/save_wifi'>");
  p += F("<label>SSID</label><input name='ssid' value='");
  p += htmlEscape(cfg.wifi_ssid);
  p += F("'>");
  p += F("<label>Passwort (leer lassen = unverändert)</label>");
  p += F("<input name='pass' type='password' value=''>");
  p += F("<button type='submit'>Speichern</button></form></div>");
  p += F("<div class='card'><form method='POST' action='/wipe_wifi' onsubmit='return confirm(\"WLAN wirklich löschen?\")'>");
  p += F("<button type='submit'>WLAN-Daten löschen (AP Setup)</button></form></div>");
  p += pageFooter();
  server.send(200, "text/html; charset=utf-8", p);
}

static void handleMqttPage() {
  String p = pageHeader("MQTT Einstellungen");
  p += F("<div class='card'><form method='POST' action='/save_mqtt'>");

  p += F("<label>Host / IP</label><input name='host' value='");
  p += htmlEscape(cfg.mqtt_host);
  p += F("'>");

  p += F("<label>Port</label><input name='port' type='number' value='");
  p += String(cfg.mqtt_port);
  p += F("'>");

  p += F("<label>User (optional)</label><input name='user' value='");
  p += htmlEscape(cfg.mqtt_user);
  p += F("'>");

  p += F("<label>Passwort (optional)</label><input name='mpass' type='password' value='");
  p += htmlEscape(cfg.mqtt_pass);
  p += F("'>");

  p += F("<label>Default-Rolle beim Boot</label><select name='role'>");
  p += F("<option value='eingang'");
  if (cfg.role == "eingang") p += F(" selected");
  p += F(">eingang</option>");
  p += F("<option value='ausgang'");
  if (cfg.role == "ausgang") p += F(" selected");
  p += F(">ausgang</option></select>");

  p += F("<p><b>Topics (fix):</b> <code>scanner/in</code>, <code>scanner/out</code>, <code>scanner/display</code>, <code>scanner/status</code></p>");

  p += F("<button type='submit'>Speichern</button></form></div>");
  p += F("<div class='card'><form method='POST' action='/mqtt_test'><button type='submit'>MQTT Verbindung testen</button></form></div>");

  p += pageFooter();
  server.send(200, "text/html; charset=utf-8", p);
}

static void handleSaveWifi() {
  if (server.method() != HTTP_POST) { server.send(405, "text/plain", "POST only"); return; }
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");
  ssid.trim();
  if (ssid.length() == 0) { server.send(400, "text/plain", "SSID fehlt"); return; }
  cfg.wifi_ssid = ssid;
  if (pass.length() > 0) cfg.wifi_pass = pass;
  saveConfig();
  server.send(200, "text/html; charset=utf-8", pageHeader("Gespeichert") + String("<div class='card'>WLAN gespeichert. Neustart...</div>") + pageFooter());
  delay(600);
  ESP.restart();
}

static void handleWipeWifi() {
  if (server.method() != HTTP_POST) { server.send(405, "text/plain", "POST only"); return; }
  cfg.wifi_ssid = "";
  cfg.wifi_pass = "";
  saveConfig();
  server.send(200, "text/html; charset=utf-8", pageHeader("WLAN geloescht") + String("<div class='card'>WLAN-Daten gelöscht. Neustart in AP...</div>") + pageFooter());
  delay(600);
  ESP.restart();
}

static void handleSaveMqtt() {
  if (server.method() != HTTP_POST) { server.send(405, "text/plain", "POST only"); return; }
  cfg.mqtt_host = server.arg("host"); cfg.mqtt_host.trim();
  String portS  = server.arg("port"); portS.trim();
  cfg.mqtt_user = server.arg("user");
  cfg.mqtt_pass = server.arg("mpass");
  cfg.role      = server.arg("role"); cfg.role.trim(); cfg.role.toLowerCase();
  if (cfg.role != "eingang" && cfg.role != "ausgang") cfg.role = "eingang";

  int p = portS.toInt();
  if (p <= 0 || p > 65535) cfg.mqtt_port = 1883;
  else cfg.mqtt_port = (uint16_t)p;

  saveConfig();

  server.send(200, "text/html; charset=utf-8", pageHeader("Gespeichert") + String("<div class='card'>MQTT gespeichert. (Reconnect automatisch)</div>") + pageFooter());
  if (mqtt.connected()) mqtt.disconnect();
}

static void handleMqttTest() {
  if (server.method() != HTTP_POST) { server.send(405, "text/plain", "POST only"); return; }
  if (cfg.mqtt_host.length() == 0) {
    server.send(200, "text/html; charset=utf-8", pageHeader("MQTT Test") + String("<div class='card'>MQTT Host ist leer.</div>") + pageFooter());
    return;
  }
  mqttConnectTry();
  String res = mqtt.connected() ? "OK (verbunden)" : "NICHT verbunden";
  server.send(200, "text/html; charset=utf-8", pageHeader("MQTT Test") + String("<div class='card'>Ergebnis: ") + res + "</div>" + pageFooter());
}

static void handleReboot() {
  server.send(200, "text/html; charset=utf-8", pageHeader("Reboot") + String("<div class='card'>Neustart...</div>") + pageFooter());
  delay(600);
  ESP.restart();
}

static void handleFactory() {
  if (LittleFS.exists(CONFIG_PATH)) LittleFS.remove(CONFIG_PATH);
  WiFi.disconnect(true);
  delay(200);
  server.send(200, "text/html; charset=utf-8", pageHeader("Factory Reset") + String("<div class='card'>Zurückgesetzt. Neustart...</div>") + pageFooter());
  delay(800);
  ESP.restart();
}

void webSetup() {
  server.on("/", handleRoot);
  server.on("/wifi", handleWifiPage);
  server.on("/mqtt", handleMqttPage);

  server.on("/save_wifi", handleSaveWifi);
  server.on("/wipe_wifi", handleWipeWifi);

  server.on("/save_mqtt", handleSaveMqtt);
  server.on("/mqtt_test", handleMqttTest);

  server.on("/reboot", handleReboot);
  server.on("/factory", handleFactory);

  server.onNotFound([]() { server.send(404, "text/plain", "Not found"); });

  server.begin();
  Serial.println("Webserver: OK");
}

void webLoop() {
  server.handleClient();
}
