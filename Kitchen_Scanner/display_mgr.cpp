#include "display_mgr.h"
#include <math.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

String displayText = "READY";
bool displayDirty = true;

// Diese Funktionen kommen aus led_mgr.cpp (Header deklariert sie)
extern uint8_t getRunnerIdx();
extern uint8_t getLedEffect();

static void drawRingFX() {
  const int cx = 64;
  const int cy = 32;
  const int r  = 28;

  for (int i = 0; i < 12; i++) {
    float a = (float)i * (2.0f * (float)M_PI / 12.0f) - (float)M_PI / 2.0f;
    int x = cx + (int)lroundf(cosf(a) * r);
    int y = cy + (int)lroundf(sinf(a) * r);
    u8g2.drawDisc(x, y, 2);
  }

  int hi = (int)(getRunnerIdx() % 12);
  if (getLedEffect() == 1 /*FLASH*/) {
    u8g2.drawCircle(cx, cy, 30);
    u8g2.drawDisc(cx, cy, 3);
  } else if (trig_holding) {
    float a = (float)hi * (2.0f * (float)M_PI / 12.0f) - (float)M_PI / 2.0f;
    int x = cx + (int)lroundf(cosf(a) * r);
    int y = cy + (int)lroundf(sinf(a) * r);
    u8g2.drawDisc(x, y, 4);
  } else {
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.drawRFrame(2, 2, 124, 60, 4);
  }
}

static void wrapTextByPixels(const String& text, int maxWidthPx, String* lines, int& lineCount) {
  lineCount = 0;

  String t = text;
  t.trim();
  if (t.length() == 0) t = "READY";

  int pos = 0;
  while (pos < (int)t.length() && lineCount < 6) {
    while (pos < (int)t.length() && t[pos] == ' ') pos++;

    int bestEnd = -1;
    int scan = pos;
    int lastSpace = -1;

    while (scan < (int)t.length()) {
      if (t[scan] == ' ') lastSpace = scan;
      String candidate = t.substring(pos, scan + 1);
      int w = u8g2.getUTF8Width(candidate.c_str());
      if (w > maxWidthPx) {
        if (lastSpace >= pos) bestEnd = lastSpace;
        else bestEnd = scan;
        break;
      }
      scan++;
    }

    if (bestEnd < 0) {
      lines[lineCount++] = t.substring(pos);
      break;
    } else {
      String line = t.substring(pos, bestEnd);
      line.trim();
      if (line.length() == 0) {
        line = t.substring(pos, min(pos + 1, (int)t.length()));
        bestEnd = pos + 1;
      }
      lines[lineCount++] = line;
      pos = bestEnd + 1;
    }
  }
}

static void chooseFontForText(const String& t) {
  String tmp = t;
  tmp.trim();
  int n = tmp.length();

  if (n <= 10) u8g2.setFont(u8g2_font_helvB18_tf);
  else if (n <= 24) u8g2.setFont(u8g2_font_helvB14_tf);
  else u8g2.setFont(u8g2_font_6x10_tf);
}

static void drawCenteredTextFancy(const String& t) {
  chooseFontForText(t);

  const int maxWidth = 96;

  String lines[6];
  int lineCount = 0;
  wrapTextByPixels(t, maxWidth, lines, lineCount);

  int ascent  = u8g2.getAscent();
  int descent = -u8g2.getDescent();
  int lineH   = ascent + descent + 2;

  int totalH = lineCount * lineH;
  int y0 = (64 - totalH) / 2 + ascent;

  for (int i = 0; i < lineCount; i++) {
    const char* s = lines[i].c_str();
    int w = u8g2.getUTF8Width(s);
    int x = (128 - w) / 2;
    int y = y0 + i * lineH;

    u8g2.drawStr(x + 1, y + 1, s);
    u8g2.drawStr(x, y, s);
  }
}

void displaySetup() {
  u8g2.begin();
  displayDirty = true;
}

void displayUpdateIfNeeded() {
  if (!displayDirty) return;
  displayDirty = false;

  u8g2.clearBuffer();
  drawRingFX();
  drawCenteredTextFancy(displayText);
  u8g2.sendBuffer();
}
