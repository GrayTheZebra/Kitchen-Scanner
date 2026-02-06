#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

static const char* CONFIG_PATH = "/config.json";

struct AppConfig {
  String wifi_ssid = "";
  String wifi_pass = "";

  String mqtt_host = "";
  uint16_t mqtt_port = 1883;
  String mqtt_user = "";
  String mqtt_pass = "";

  String role = "eingang"; // eingang|ausgang
};

extern AppConfig cfg;

bool loadConfig();
bool saveConfig();
String jsonStringOrDefault(JsonVariant v, const char* def);
