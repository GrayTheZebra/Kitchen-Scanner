#pragma once
#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "app_config.h"
#include "topics.h"

extern WiFiClient espClient;
extern PubSubClient mqtt;

void mqttSetup();
void mqttLoop();
void mqttConnectTry();
void mqttSeedTopics();

extern uint32_t last_mqtt_try_ms;

// vom Display genutzt
void mqttCallback(char* topic, byte* payload, unsigned int length);

// Hook für OLED-Text
extern String displayText;
extern bool displayDirty;
