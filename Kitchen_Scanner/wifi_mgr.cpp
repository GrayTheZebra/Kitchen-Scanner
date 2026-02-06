#include "wifi_mgr.h"

bool wifi_ap_mode = false;

void startAP() {
  wifi_ap_mode = true;
  WiFi.mode(WIFI_AP);
  if (strlen(AP_PASS) >= 8) WiFi.softAP(AP_SSID, AP_PASS);
  else WiFi.softAP(AP_SSID);

  Serial.print("AP aktiv. IP: ");
  Serial.println(WiFi.softAPIP());
}

void startSTAOrFallback() {
  wifi_ap_mode = false;

  if (cfg.wifi_ssid.length() == 0) {
    Serial.println("Kein WLAN konfiguriert -> AP Modus");
    startAP();
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(cfg.wifi_ssid.c_str(), cfg.wifi_pass.c_str());

  Serial.print("WLAN verbinden: ");
  Serial.println(cfg.wifi_ssid);

  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - t0) < WIFI_CONNECT_TIMEOUT_MS) {
    delay(250);
    yield();
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WLAN OK, IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WLAN Timeout -> AP Fallback");
    startAP();
  }
}
