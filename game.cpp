#include "game.h"
#include "gfx.h"
#include "ground.h"
#include "mountains.h"
#include "clouds.h"
#include "config.h"
static uint32_t lastFrame = 0;

void gameInit() {
  gfxInit();
  gfxClearSky();
  cloudsInit();
  mountainsInit();
  groundInit();
  lastFrame = millis();
}

void gameUpdate() {
  uint32_t now = millis();
  uint32_t dt  = now - lastFrame;
  if (dt < FRAME_MS) return;
  lastFrame = now;

  // Update & render parallax background
  cloudsUpdate(dt);
  mountainsUpdate(dt);
  cloudsRender();
  mountainsRender();

  // Update & render ground (scrolling left→right)
  groundUpdate(dt);
  groundRender();
}
