#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

extern Adafruit_NeoPixel pixels;

extern bool trig_holding;
extern uint8_t led_runner_idx;
extern uint8_t led_effect;

void ledSetup();
void ledUpdate();

void ledStartFlash(uint32_t ms);
void ledStartWipe(uint32_t total_ms);

uint32_t modeIdleColor();
void setScanMode(uint8_t mode); // 0=in,1=out

// Export für Display-Ring-FX
uint8_t getRunnerIdx();
uint8_t getLedEffect();
