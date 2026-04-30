#include <Arduino.h>
#include <TFT_eSPI.h>

// animation frames
#include "sprites/mametchi_idle1.h"
#include "sprites/mametchi_idle2.h"

// sprite dimensions -- all of them should be the same
int spriteW = MAMETCHI_IDLE1_W;
int spriteH = MAMETCHI_IDLE1_H;

TFT_eSPI tft = TFT_eSPI();

#define SCREEN_W 160
#define SCREEN_H 80

#define FRAME_DELAY 500
int currentFrame = 0;
unsigned long lastFrameTime = 0;

const uint16_t* frames[] = {mametchi_idle1, mametchi_idle2};

void drawFrame(int frameIdx) {
  int x = (SCREEN_W - spriteW) / 2;
  int y = (SCREEN_H - spriteH) / 2;

  tft.fillScreen(TFT_WHITE);
  tft.pushImage(x, y, spriteW, spriteH, frames[frameIdx], 0x0000);
}

void setup() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);
  tft.setSwapBytes(true);

  // tft.fillRect(10, 10, 50, 50, TFT_RED);
  // delay(1000);

  drawFrame(0);
  lastFrameTime = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (millis() - lastFrameTime > FRAME_DELAY) {
    currentFrame = (currentFrame + 1) % (sizeof(frames) / sizeof(frames[0]));
    drawFrame(currentFrame);
    lastFrameTime = millis();
  }
}


