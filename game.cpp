#include "game.h"
#include "gfx.h"
#include "ground.h"
#include "mountains.h"
#include "clouds.h"
#include "tornado.h"
#include "config.h"
static uint32_t lastFrame = 0;

void gameInit() {
  gfxInit();
  gfxClearSky();
  cloudsInit();
  mountainsInit();
  tornadoInit();
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
  tornadoUpdate(dt);
  cloudsRender();
  mountainsRender();
  tornadoRender();

  // Update & render ground (scrolling left→right)
  groundUpdate(dt);
  groundRender();
}
