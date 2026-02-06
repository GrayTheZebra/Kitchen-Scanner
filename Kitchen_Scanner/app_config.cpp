#include "app_config.h"

AppConfig cfg;

String jsonStringOrDefault(JsonVariant v, const char* def) {
  if (v.is<const char*>()) return String(v.as<const char*>());
  return String(def);
}

bool saveConfig() {
  StaticJsonDocument<512> doc;
  doc["wifi_ssid"] = cfg.wifi_ssid;
  doc["wifi_pass"] = cfg.wifi_pass;

  doc["mqtt_host"] = cfg.mqtt_host;
  doc["mqtt_port"] = cfg.mqtt_port;
  doc["mqtt_user"] = cfg.mqtt_user;
  doc["mqtt_pass"] = cfg.mqtt_pass;

  doc["role"] = cfg.role;

  File f = LittleFS.open(CONFIG_PATH, "w");
  if (!f) return false;
  if (serializeJson(doc, f) == 0) { f.close(); return false; }
  f.close();
  return true;
}

bool loadConfig() {
  if (!LittleFS.exists(CONFIG_PATH)) return false;
  File f = LittleFS.open(CONFIG_PATH, "r");
  if (!f) return false;

  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) return false;

  cfg.wifi_ssid = jsonStringOrDefault(doc["wifi_ssid"], "");
  cfg.wifi_pass = jsonStringOrDefault(doc["wifi_pass"], "");

  cfg.mqtt_host = jsonStringOrDefault(doc["mqtt_host"], "");
  cfg.mqtt_user = jsonStringOrDefault(doc["mqtt_user"], "");
  cfg.mqtt_pass = jsonStringOrDefault(doc["mqtt_pass"], "");

  if (doc["mqtt_port"].is<uint16_t>()) cfg.mqtt_port = doc["mqtt_port"];
  else cfg.mqtt_port = 1883;

  cfg.role = jsonStringOrDefault(doc["role"], "eingang");
  cfg.role.trim();
  cfg.role.toLowerCase();
  if (cfg.role != "eingang" && cfg.role != "ausgang") cfg.role = "eingang";

  return true;
}
