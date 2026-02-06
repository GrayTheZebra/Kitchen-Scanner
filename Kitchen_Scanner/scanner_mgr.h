#pragma once
#include <Arduino.h>
#include <SoftwareSerial.h>

void scannerSetup();
void processScanner();
void publishScan(const String& code);

uint8_t getScanMode();      // 0=in,1=out
void toggleModeIfAllowed(); // Button-Logik ruft das auf

const char* modeToText();
const char* modeToSource();
const char* actionToText();
const char* currentTopic();
