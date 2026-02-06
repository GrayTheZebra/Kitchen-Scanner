#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>

extern Adafruit_VL53L0X tof;
extern bool tof_ok;

extern bool trig_holding;
extern bool wait_for_far;
extern uint32_t trig_hold_start_ms;

void tofSetup();
void processToF();

void triggerHoldStart();
void triggerHoldStop(const char* reason);

static const uint8_t TRIG_PIN = D5;
