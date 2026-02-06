#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "app_config.h"

static const uint32_t WIFI_CONNECT_TIMEOUT_MS = 20000;
static const char* AP_SSID = "KitchenScanner-Setup";
static const char* AP_PASS = "";

extern bool wifi_ap_mode;

void startAP();
void startSTAOrFallback();
