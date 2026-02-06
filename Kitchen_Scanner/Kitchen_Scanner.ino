#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>

#include "app_config.h"
#include "wifi_mgr.h"
#include "mqtt_mgr.h"
#include "web_ui.h"
#include "led_mgr.h"
#include "display_mgr.h"
#include "tof_mgr.h"
#include "scanner_mgr.h"

// Button
static const uint8_t MODE_BTN_PIN = D7;
static const uint32_t BTN_DEBOUNCE_MS = 35;
static bool btn_last_raw = true;
static bool btn_stable = true;
static uint32_t btn_last_change_ms = 0;

static void buttonSetup() {
  pinMode(MODE_BTN_PIN, INPUT_PULLUP);
  btn_last_raw = digitalRead(MODE_BTN_PIN);
  btn_stable = btn_last_raw;
  btn_last_change_ms = millis();
}

static void buttonLoop() {
  bool raw = digitalRead(MODE_BTN_PIN);

  if (raw != btn_last_raw) {
    btn_last_raw = raw;
    btn_last_change_ms = millis();
  }

  if ((millis() - btn_last_change_ms) >= BTN_DEBOUNCE_MS && raw != btn_stable) {
    btn_stable = raw;
    if (btn_stable == LOW) {
      toggleModeIfAllowed();
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("=== Kitchen Scanner (Topics: scanner/in,out,display,status) ===");

  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);

  ledSetup();

  if (!LittleFS.begin()) {
    Serial.println("LittleFS Mount fehlgeschlagen!");
  } else {
    loadConfig();
  }

  buttonSetup();
  scannerSetup();
  tofSetup();
  displaySetup();
  mqttSetup();

  startSTAOrFallback();
  webSetup();

  if (WiFi.status() == WL_CONNECTED && cfg.mqtt_host.length() > 0) {
    mqttConnectTry();
  }

  ledUpdate();
  displayUpdateIfNeeded();
}

void loop() {
  webLoop();
  yield();

  buttonLoop();
  mqttLoop();

  processToF();
  processScanner();

  displayUpdateIfNeeded();
  ledUpdate();

  yield();
}
