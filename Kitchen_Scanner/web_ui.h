#pragma once
#include <Arduino.h>
#include <ESP8266WebServer.h>

extern ESP8266WebServer server;

void webSetup();
void webLoop();

// util
String htmlEscape(const String& s);
