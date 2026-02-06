#pragma once
#include <Arduino.h>
#include <U8g2lib.h>

extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;

extern String displayText;
extern bool displayDirty;

void displaySetup();
void displayUpdateIfNeeded();

// LED state für RingFX
uint8_t getRunnerIdx();
uint8_t getLedEffect();
extern bool trig_holding;
