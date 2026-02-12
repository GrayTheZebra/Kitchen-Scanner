#include "scanner_mgr.h"
#include "mqtt_mgr.h"
#include "topics.h"
#include "app_config.h"
#include "led_mgr.h"
#include "display_mgr.h"
#include "tof_mgr.h" // triggerHoldStop, wait_for_far

static const uint8_t SCANNER_RX_PIN = D6;
static const int SCANNER_BAUD = 9600;
static SoftwareSerial scannerSerial(SCANNER_RX_PIN, -1);

static const uint32_t DUP_MS = 700;
static const uint8_t MIN_CODE_LEN = 6;
static const uint8_t MAX_CODE_LEN = 64;
static const uint8_t MAX_BUFFER_LEN = 128;
static const uint16_t MODE_WIPE_MS = 900;
static const uint16_t SCAN_FLASH_MS = 120;

static String lastCode;
static uint32_t lastScanMs = 0;

enum ScanMode : uint8_t { MODE_IN = 0, MODE_OUT = 1 };
static ScanMode scanMode = MODE_IN;

static bool isAllowedCodeChar(char c) {
  return (
    (c >= '0' && c <= '9') ||
    (c >= 'A' && c <= 'Z') ||
    (c >= 'a' && c <= 'z') ||
    c == '-' || c == '_' || c == '.' || c == ':' || c == '/'
  );
}

static void applyModeFromCfg() {
  scanMode = (cfg.role == "ausgang") ? MODE_OUT : MODE_IN;
  setScanMode((uint8_t)scanMode);
}

const char* modeToText() {
  return (scanMode == MODE_IN) ? "hinzufuegen (IN)" : "entfernen (OUT)";
}

const char* modeToSource() {
  return (scanMode == MODE_OUT) ? "ausgang" : "eingang";
}

const char* actionToText() {
  return (scanMode == MODE_IN) ? "in" : "out";
}

const char* currentTopic() {
  return (scanMode == MODE_OUT) ? TOPIC_OUT : TOPIC_IN;
}

uint8_t getScanMode() { return (uint8_t)scanMode; }

void scannerSetup() {
  scannerSerial.begin(SCANNER_BAUD);
  applyModeFromCfg();

  Serial.print("[MODE] Start: ");
  Serial.println(modeToText());
}

void toggleModeIfAllowed() {
  if (trig_holding) {
    Serial.println("[MODE] Ignoriere Toggle (Scanner gerade HOLD)");
    return;
  }

  scanMode = (scanMode == MODE_IN) ? MODE_OUT : MODE_IN;
  setScanMode((uint8_t)scanMode);

  Serial.print("[MODE] ");
  Serial.println(modeToText());

  ledStartWipe(MODE_WIPE_MS);
  displayText = (scanMode == MODE_IN) ? "IN" : "OUT";
  displayDirty = true;
}

static bool looksLikeCode(const String& code) {
  if (code.length() < MIN_CODE_LEN || code.length() > MAX_CODE_LEN) return false;

  for (size_t i = 0; i < code.length(); i++) {
    if (!isAllowedCodeChar(code[i])) return false;
  }

  return true;
}

static String buildPayload(const String& code, uint32_t uptimeMs) {
  String payload = "{";
  payload += "\"ean\":\"" + code + "\",";
  payload += "\"qty\":1,";
  payload += "\"action\":\"" + String(actionToText()) + "\",";
  payload += "\"source\":\"" + String(modeToSource()) + "\",";
  payload += "\"uptime_ms\":" + String(uptimeMs);
  payload += "}";
  return payload;
}

static void processScannerBuffer(const String& bufferContent) {
  if (looksLikeCode(bufferContent)) {
    publishScan(bufferContent);
    return;
  }

  Serial.print("[WARN] Unplausibel: ");
  Serial.println(bufferContent);
}

void publishScan(const String& code) {
  uint32_t now = millis();

  if (code == lastCode && (now - lastScanMs) < DUP_MS) {
    Serial.println("[SKIP] Doppelscan ignoriert");
    return;
  }

  lastCode = code;
  lastScanMs = now;

  Serial.print("[SCAN] ");
  Serial.print(code);
  Serial.print(" -> ");
  Serial.println(modeToText());

  ledStartFlash(SCAN_FLASH_MS);
  displayDirty = true;

  triggerHoldStop("scan");
  wait_for_far = true;

  if (cfg.mqtt_host.length() == 0) return;
  if (!mqtt.connected()) return;

  String payload = buildPayload(code, now);
  mqtt.publish(currentTopic(), payload.c_str(), false);
}

void processScanner() {
  static String buf;

  while (scannerSerial.available() > 0) {
    char c = (char)scannerSerial.read();

    if (c == '\n' || c == '\r') {
      if (buf.length() > 0) {
        String completeScan = buf;
        buf = "";
        processScannerBuffer(completeScan);
      }
      continue;
    }

    if (c >= 32 && c <= 126 && buf.length() < MAX_BUFFER_LEN) {
      buf += c;
    }
  }
}
