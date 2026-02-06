#include "tof_mgr.h"
#include "display_mgr.h" // displayDirty
#include "led_mgr.h"     // trig_holding

Adafruit_VL53L0X tof;
bool tof_ok = false;

static const uint8_t TOF_SDA = D2;
static const uint8_t TOF_SCL = D1;

static const uint16_t TOF_NEAR_MM = 120;
static const uint16_t TOF_FAR_MM  = 170;
static const uint32_t TRIG_MAX_HOLD_MS = 6000;

uint32_t trig_hold_start_ms = 0;
bool wait_for_far = false;

void triggerHoldStart() {
  if (trig_holding) return;
  trig_holding = true;
  trig_hold_start_ms = millis();
  digitalWrite(TRIG_PIN, HIGH);
  Serial.println("[TRIG] HOLD ON");
  displayDirty = true;
}

void triggerHoldStop(const char* reason) {
  if (!trig_holding) return;
  digitalWrite(TRIG_PIN, LOW);
  trig_holding = false;
  Serial.print("[TRIG] HOLD OFF (");
  Serial.print(reason);
  Serial.println(")");
  displayDirty = true;
}

void tofSetup() {
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);

  Wire.begin(TOF_SDA, TOF_SCL);
  Wire.setClock(100000);

  tof_ok = tof.begin();
  if (tof_ok) Serial.println("✅ VL53L0X bereit");
  else Serial.println("❌ VL53L0X nicht gefunden (ToF deaktiviert)");
}

void processToF() {
  if (!tof_ok) return;

  VL53L0X_RangingMeasurementData_t measure;
  tof.rangingTest(&measure, false);

  bool valid = (measure.RangeStatus == 0);
  uint16_t range = valid ? measure.RangeMilliMeter : 9999;

  if (!valid) {
    if (trig_holding && (millis() - trig_hold_start_ms) > TRIG_MAX_HOLD_MS) {
      triggerHoldStop("timeout");
      wait_for_far = true;
    }
    triggerHoldStop("far");
    return;
  }

  if (range > TOF_FAR_MM) {
    if (wait_for_far) {
      wait_for_far = false;
      Serial.println("[ARM] Objekt weg -> wieder bereit");
      displayDirty = true;
    }
    triggerHoldStop("far");
    return;
  }

  if (range < TOF_NEAR_MM && !wait_for_far) {
    triggerHoldStart();
  }

  if (trig_holding && (millis() - trig_hold_start_ms) > TRIG_MAX_HOLD_MS) {
    triggerHoldStop("timeout");
    wait_for_far = true;
  }
}
