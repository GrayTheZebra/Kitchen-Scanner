#include "led_mgr.h"

static const uint8_t PIXEL_PIN = D4;
static const uint8_t PIXEL_COUNT = 12;
Adafruit_NeoPixel pixels(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

static const uint8_t BRIGHT_IDLE = 18;
static const uint8_t BRIGHT_RUN  = 16;
static const uint8_t BRIGHT_RUN_HALO1 = 6;
static const uint8_t BRIGHT_RUN_HALO2 = 4;
static const uint8_t BRIGHT_FLASH = 160;

static const uint8_t LED_EFF_NONE  = 0;
static const uint8_t LED_EFF_FLASH = 1;
static const uint8_t LED_EFF_WIPE  = 2;

uint8_t led_effect = LED_EFF_NONE;
uint32_t led_effect_until_ms = 0;

uint8_t wipe_pos = 0;
uint32_t wipe_next_step_ms = 0;

uint32_t led_last_frame_ms = 0;
uint8_t led_runner_idx = 0;

bool trig_holding = false;

static uint8_t scanMode = 0; // 0=in, 1=out

static uint32_t col(uint8_t r, uint8_t g, uint8_t b) { return pixels.Color(r, g, b); }
static void pixelsAll(uint32_t c) { for (uint8_t i=0;i<PIXEL_COUNT;i++) pixels.setPixelColor(i,c); }
static void pixelsClear() { pixelsAll(col(0,0,0)); }

void setScanMode(uint8_t mode) { scanMode = mode ? 1 : 0; }

uint32_t modeIdleColor() {
  if (scanMode == 0) return col(0, BRIGHT_IDLE, 0);
  return col(BRIGHT_IDLE, 0, 0);
}

void ledStartFlash(uint32_t ms) {
  led_effect = LED_EFF_FLASH;
  led_effect_until_ms = millis() + ms;
}

void ledStartWipe(uint32_t total_ms) {
  led_effect = LED_EFF_WIPE;
  wipe_pos = 0;
  wipe_next_step_ms = millis();
  led_effect_until_ms = millis() + total_ms;
}

static void ledDrawIdle() { pixelsAll(modeIdleColor()); }

static void ledDrawRunnerOverlay() {
  uint8_t idx  = led_runner_idx % PIXEL_COUNT;
  uint8_t prev = (idx + PIXEL_COUNT - 1) % PIXEL_COUNT;
  uint8_t next = (idx + 1) % PIXEL_COUNT;

  pixels.setPixelColor(prev, col(BRIGHT_RUN_HALO1, BRIGHT_RUN_HALO1, BRIGHT_RUN_HALO1));
  pixels.setPixelColor(idx,  col(BRIGHT_RUN,        BRIGHT_RUN,        BRIGHT_RUN));
  pixels.setPixelColor(next, col(BRIGHT_RUN_HALO2, BRIGHT_RUN_HALO2, BRIGHT_RUN_HALO2));
}

void ledSetup() {
  pixels.begin();
  pixelsClear();
  pixels.show();
}

void ledUpdate() {
  uint32_t now = millis();
  if (now - led_last_frame_ms < 50) return;
  led_last_frame_ms = now;

  if (trig_holding) {
    led_runner_idx = (led_runner_idx + 1) % PIXEL_COUNT;
  }

  if (led_effect != LED_EFF_NONE && (int32_t)(now - led_effect_until_ms) >= 0) {
    led_effect = LED_EFF_NONE;
  }

  if (led_effect == LED_EFF_FLASH) {
    pixelsAll(col(BRIGHT_FLASH, BRIGHT_FLASH, BRIGHT_FLASH));
    pixels.show();
    return;
  }

  if (led_effect == LED_EFF_WIPE) {
    ledDrawIdle();

    uint8_t highlight = 60;
    uint32_t hiCol = (scanMode == 0) ? col(0, highlight, 0) : col(highlight, 0, 0);

    if ((int32_t)(now - wipe_next_step_ms) >= 0) {
      wipe_next_step_ms = now + 55;
      if (wipe_pos < PIXEL_COUNT) wipe_pos++;
    }

    for (uint8_t i = 0; i < wipe_pos && i < PIXEL_COUNT; i++) {
      pixels.setPixelColor(i, hiCol);
    }

    pixels.show();
    return;
  }

  ledDrawIdle();
  if (trig_holding) ledDrawRunnerOverlay();
  pixels.show();
}

uint8_t getRunnerIdx() { return led_runner_idx; }
uint8_t getLedEffect() { return led_effect; }
