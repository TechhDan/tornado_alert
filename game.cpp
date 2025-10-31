#include "game.h"
#include "gfx.h"
#include "config.h"
#include "colors.h"

static int x = 40, y = 40;
static int vx = 2, vy = 2;
static uint32_t lastFrame = 0;

void gameInit() {
  gfxInit();
  gfxClearSky();
  groundInit();
  //spriteBuild();
}

void gameUpdate() {
  uint32_t now = millis();
  uint32_t dt  = now - lastFrame;
  if (dt < FRAME_MS) return;
  lastFrame = now;

  // Update & render ground (scrolling left→right)
  groundUpdate(dt);
  groundRender();

  // (We’ll add character drawing next—after ground so the player appears “above”.)
}
