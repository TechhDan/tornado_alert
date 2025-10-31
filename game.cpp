#include "game.h"
#include "gfx.h"
#include "ground.h"
#include "mountains.h"
#include "config.h"
#include "colors.h"

static const int SPRITE_SIZE = 16;

static float posX = 40.0f;
static float posY = 80.0f;
static float velX = 70.0f;  // pixels per second
static float velY = -50.0f; // pixels per second

static int lastDrawX = -1;
static int lastDrawY = -1;

static uint32_t lastFrame = 0;

void gameInit() {
  gfxInit();
  gfxClearSky();
  mountainsInit();
  groundInit();
  spriteBuild();
  lastFrame = millis();
}

void gameUpdate() {
  uint32_t now = millis();
  uint32_t dt  = now - lastFrame;
  if (dt < FRAME_MS) return;
  lastFrame = now;

  if (lastDrawX >= 0 && lastDrawY >= 0) {
    spriteErase(lastDrawX, lastDrawY, SPRITE_SIZE, SPRITE_SIZE);
  }

  // Update & render parallax background
  mountainsUpdate(dt);
  mountainsRender();

  // Update & render ground (scrolling left→right)
  groundUpdate(dt);
  groundRender();

  float dtSeconds = dt / 1000.0f;

  posX += velX * dtSeconds;
  posY += velY * dtSeconds;

  const float minX = 0.0f;
  const float maxX = static_cast<float>(SCREEN_W - SPRITE_SIZE);
  if (posX < minX) {
    posX = minX;
    velX = -velX;
  } else if (posX > maxX) {
    posX = maxX;
    velX = -velX;
  }

  const float minY = 16.0f;
  const float maxY = static_cast<float>(SCREEN_H - 48 - SPRITE_SIZE); // stay above ground
  if (posY < minY) {
    posY = minY;
    velY = -velY;
  } else if (posY > maxY) {
    posY = maxY;
    velY = -velY;
  }

  lastDrawX = static_cast<int>(posX);
  lastDrawY = static_cast<int>(posY);

  spriteDraw(lastDrawX, lastDrawY, C_CLEAR(tft));
}
