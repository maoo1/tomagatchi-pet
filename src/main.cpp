#include <Arduino.h>
#include <TFT_eSPI.h>

// animation frames
#include "sprites/mametchi_idle1.h"
#include "sprites/mametchi_idle2.h"
#include "sprites/mametchi_eating1.h"
#include "sprites/mametchi_eating2.h"
#include "sprites/mametchi_happy.h"

TFT_eSPI tft = TFT_eSPI();

#define SCREEN_W 135
#define SCREEN_H 240
#define FRAME_DELAY 500

// sprite dimensions -- all of them should be the same
int spriteW = MAMETCHI_IDLE1_W;
int spriteH = MAMETCHI_IDLE1_H;

// STATS
#define MAX_STAT        10
#define STAT_DECAY_MS   60000UL   // 1 min

int hunger    = MAX_STAT;
int happiness = MAX_STAT;
 
unsigned long lastDecayTime = 0;
 
// PET STATES
enum PetState { STATE_IDLE, STATE_EATING, STATE_HAPPY };
PetState petState = STATE_IDLE;
 
unsigned long stateStartTime  = 0;
#define STATE_DURATION_MS 3000  // eating/happy animation lasts 3s

// ANIMATION FRAMES
#define FRAME_DELAY 500
unsigned long lastFrameTime = 0;
int currentFrame = 0;
uint16_t frameBuf[MAMETCHI_IDLE1_W * MAMETCHI_IDLE1_H];
 
const uint16_t* idleFrames[]   = { mametchi_idle1,   mametchi_idle2   };
const uint16_t* eatingFrames[] = { mametchi_eating1, mametchi_eating2 };
const uint16_t* happyFrames[]  = { mametchi_happy,   mametchi_happy   };

//UI LAYOUT
#define BAR_X       10
#define BAR_Y_HUN   10
#define BAR_Y_HAP   30
#define BAR_W       115
#define BAR_H       12
#define BAR_PADDING 2
 
#define SPRITE_X  ((SCREEN_W - spriteW) / 2)
#define SPRITE_Y  90
 
// Button pins (TTGO T-Display)
#define BTN_FEED  0   // GPIO0  = left button  → feed
#define BTN_PLAY  35  // GPIO35 = right button → play


void drawSprite(const uint16_t* pgmData);
void drawBars();
void drawButtons();
void setState(PetState s);
void updateState();

void drawSprite(const uint16_t* pgmData) {
  memcpy_P(frameBuf, pgmData, spriteW * spriteH * sizeof(uint16_t));
  tft.pushImage(SPRITE_X, SPRITE_Y, spriteW, spriteH, frameBuf);
}

void drawBar(int x, int y, int value, uint16_t fillColor, const char* label) {
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(x, y + 2);
  tft.print(label);
 
  int labelW = 20;
  int barX = x + labelW;
  int barW = BAR_W - labelW;
 
  tft.fillRect(barX, y, barW, BAR_H, TFT_LIGHTGREY);
 
  int fillW = (int)((float)value / MAX_STAT * (barW - BAR_PADDING * 2));
  if (fillW > 0) {
    tft.fillRect(barX + BAR_PADDING, y + BAR_PADDING,
                 fillW, BAR_H - BAR_PADDING * 2, fillColor);
  }
 
  tft.drawRect(barX, y, barW, BAR_H, TFT_LIGHTGREY);
 
  tft.setCursor(barX + barW + 3, y + 2);
  // tft.print(value);
}

void drawBars() {
  uint16_t hungerColor    = (hunger    <= 3) ? TFT_RED : TFT_BLACK;
  uint16_t happinessColor = (happiness <= 3) ? TFT_RED : TFT_BLACK;
  drawBar(BAR_X, BAR_Y_HUN, hunger,    hungerColor,    "HUN");
  drawBar(BAR_X, BAR_Y_HAP, happiness, happinessColor, "HAP");
}

void drawButtons() {
  tft.setTextColor(TFT_DARKGREY, TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, SCREEN_H - 20);
  tft.print("[FEED]");
  tft.setCursor(90, SCREEN_H - 20);
  tft.print("[PLAY]");
}

void setState(PetState s) {
  petState       = s;
  stateStartTime = millis();
  currentFrame   = 0;
}

void updateState() {
  if (petState != STATE_IDLE) {
    if (millis() - stateStartTime >= STATE_DURATION_MS) {
      setState(STATE_IDLE);
      tft.fillScreen(TFT_WHITE);
      drawBars();
      drawButtons();
    }
  }
}

void drawFrame(int frameIdx)
{
  // Copy sprite from PROGMEM into RAM buffer first
  memcpy_P(frameBuf, idleFrames[frameIdx], spriteW * spriteH * sizeof(uint16_t));

  int x = (SCREEN_W - spriteW) / 2;
  int y = (SCREEN_H - spriteH) / 2;

  tft.fillScreen(TFT_WHITE);
  // Pass RAM buffer to pushImage, not the PROGMEM pointer
  tft.pushImage(x, y, spriteW, spriteH, frameBuf);
}

void setup() {
  Serial.begin(115200);
 
  pinMode(BTN_FEED, INPUT_PULLUP);
  pinMode(BTN_PLAY, INPUT_PULLUP);
 
  tft.init();
  tft.setRotation(0);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_WHITE);
 
  drawBars();
  drawButtons();
  drawSprite(idleFrames[0]);
 
  lastFrameTime = millis();
  lastDecayTime = millis();
 
  Serial.println("Tamagotchi ready!");
}
void loop() {
  unsigned long now = millis();
 
  // Stat decay every 1 minute
  if (now - lastDecayTime >= STAT_DECAY_MS) {
    lastDecayTime = now;
    if (hunger    > 0) hunger--;
    if (happiness > 0) happiness--;
    drawBars();
    Serial.printf("Decay → hunger: %d, happiness: %d\n", hunger, happiness);
  }
 
  // Button: Feed
  if (digitalRead(BTN_FEED) == LOW) {
    hunger = min(hunger + 3, MAX_STAT);
    drawBars();
    setState(STATE_EATING);
    tft.fillRect(SPRITE_X, SPRITE_Y, spriteW, spriteH, TFT_WHITE);
    Serial.printf("Fed → hunger: %d\n", hunger);
    delay(200);
  }
 
  // Button: Play
  if (digitalRead(BTN_PLAY) == LOW) {
    happiness = min(happiness + 3, MAX_STAT);
    drawBars();
    setState(STATE_HAPPY);
    tft.fillRect(SPRITE_X, SPRITE_Y, spriteW, spriteH, TFT_WHITE);
    Serial.printf("Played → happiness: %d\n", happiness);
    delay(200);
  }
 
  updateState();
 
  // Animation tick
  if (now - lastFrameTime >= FRAME_DELAY) {
    lastFrameTime = now;
 
    const uint16_t** activeFrames;
    int frameCount;
 
    switch (petState) {
      case STATE_EATING:
        activeFrames = eatingFrames;
        frameCount   = 2;
        break;
      case STATE_HAPPY:
        activeFrames = happyFrames;
        frameCount   = 1;
        break;
      default:
        activeFrames = idleFrames;
        frameCount   = 2;
        break;
    }
 
    currentFrame = (currentFrame + 1) % frameCount;
    drawSprite(activeFrames[currentFrame]);
  }
}