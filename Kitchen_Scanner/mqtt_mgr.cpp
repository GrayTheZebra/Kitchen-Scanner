#include "mqtt_mgr.h"

WiFiClient espClient;
PubSubClient mqtt(espClient);

uint32_t last_mqtt_try_ms = 0;

void mqttSeedTopics() {
  if (!mqtt.connected()) return;
  mqtt.publish(TOPIC_STATUS, "online", true);
  mqtt.publish(TOPIC_DISPLAY, "READY", true);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  if (String(topic) != TOPIC_DISPLAY) return;

  String s;
  s.reserve(length + 1);
  for (unsigned int i = 0; i < length; i++) {
    char c = (char)payload[i];
    if (c == '\r' || c == '\n' || c == '\t') { s += ' '; }
    else if ((uint8_t)c < 32) { /* skip */ }
    else { s += c; }
  }
  s.trim();
  if (s.length() == 0) s = "READY";

  displayText = s;
  displayDirty = true;
}

void mqttSetup() {
  mqtt.setCallback(mqttCallback);
}

void mqttConnectTry() {
  if (cfg.mqtt_host.length() == 0) return;
  if (WiFi.status() != WL_CONNECTED) return;

  mqtt.setServer(cfg.mqtt_host.c_str(), cfg.mqtt_port);
  mqtt.setSocketTimeout(2);
  mqtt.setKeepAlive(15);
  espClient.setTimeout(2000);

  String clientId = "scanner-" + String(ESP.getChipId(), HEX);

  const char* willTopic = TOPIC_STATUS;
  const char* willMsg   = "offline";

  bool ok;
  if (cfg.mqtt_user.length() > 0) {
    ok = mqtt.connect(clientId.c_str(),
                      cfg.mqtt_user.c_str(),
                      cfg.mqtt_pass.c_str(),
                      willTopic, 0, true, willMsg);
  } else {
    ok = mqtt.connect(clientId.c_str(), nullptr, nullptr,
                      willTopic, 0, true, willMsg);
  }

  if (ok) {
    Serial.println("MQTT OK");
    mqtt.subscribe(TOPIC_IN);
    mqtt.subscribe(TOPIC_OUT);
    mqtt.subscribe(TOPIC_DISPLAY);
    mqttSeedTopics();
  } else {
    Serial.print("MQTT rc=");
    Serial.println(mqtt.state());
  }
}

void mqttLoop() {
  if (WiFi.status() == WL_CONNECTED && cfg.mqtt_host.length() > 0) {
    if (!mqtt.connected()) {
      uint32_t now = millis();
      if (now - last_mqtt_try_ms > 5000) {
        last_mqtt_try_ms = now;
        mqttConnectTry();
      }
    } else {
      mqtt.loop();
    }
  }
}
