#include <Arduino.h>
#include <TFT_eSPI.h>

// animation frames
#include "sprites/mametchi_idle1.h"
#include "sprites/mametchi_idle2.h"

#define SCREEN_W 240
#define SCREEN_H 135
#define FRAME_DELAY 500

// sprite dimensions -- all of them should be the same
int spriteW = MAMETCHI_IDLE1_W;
int spriteH = MAMETCHI_IDLE1_H;

TFT_eSPI tft = TFT_eSPI();

const uint16_t *frames[] = {mametchi_idle1, mametchi_idle2};
const int FRAME_COUNT = sizeof(frames) / sizeof(frames[0]);

int currentFrame = 0;
unsigned long lastFrameTime = 0;
uint16_t frameBuf[MAMETCHI_IDLE1_W * MAMETCHI_IDLE1_H];

// void drawFrame(int frameIdx)
// {
//   int x = (SCREEN_W - spriteW) / 2;
//   int y = (SCREEN_H - spriteH) / 2;

//   int pixelCount = spriteW * spriteH;
//   uint16_t *buf = (uint16_t *)malloc(pixelCount * sizeof(uint16_t));
//   if (!buf)
//     return; // safety check

//   memcpy_P(buf, frames[frameIdx], pixelCount * sizeof(uint16_t));

//   tft.fillScreen(TFT_WHITE);
//   tft.pushImage(x, y, spriteW, spriteH, buf, 0x0000);
//   free(buf);
// }

void drawFrame(int frameIdx)
{
  // Copy sprite from PROGMEM into RAM buffer first
  memcpy_P(frameBuf, frames[frameIdx], spriteW * spriteH * sizeof(uint16_t));

  int x = (SCREEN_W - spriteW) / 2;
  int y = (SCREEN_H - spriteH) / 2;

  tft.fillScreen(TFT_WHITE);
  // Pass RAM buffer to pushImage, not the PROGMEM pointer
  tft.pushImage(x, y, spriteW, spriteH, frameBuf);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting...");

  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_WHITE);

  Serial.println("Display init done");

  drawFrame(0);
  lastFrameTime = millis();

  Serial.println("First frame drawn");
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (millis() - lastFrameTime > FRAME_DELAY)
  {
    currentFrame = (currentFrame + 1) % FRAME_COUNT;
    drawFrame(currentFrame);
    lastFrameTime = millis();
  }
}
