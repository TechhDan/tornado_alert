// game.cpp
#include "game.h"
#include "gfx.h"
#include "ground.h"
#include "mountains.h"
#include "clouds.h"
#include "tornado.h"
#include "config.h"
#include "colors.h"

static uint32_t lastFrame = 0;

void gameInit() {
  gfxInit();                  // make sure this calls tft.setRotation(1)

  // Paint a valid first frame (sky + ground) so there are no leftovers
  tft.fillScreen(SKY_BLUE(tft));
  groundInit();
  groundRender();

  // Init clouds (mountains/tornado can be enabled later)
  cloudsInit();
  mountainsInit();
  tornadoInit();

  lastFrame = millis();
}

void gameUpdate() {
  uint32_t now = millis();
  uint32_t dt  = now - lastFrame;
  if (dt < FRAME_MS) return;
  lastFrame = now;

  // Update
  cloudsUpdate(dt);
  mountainsUpdate(dt);
  tornadoUpdate(dt);
  groundUpdate(dt);

  // Draw one frame. Do NOT clear the sky here (avoids flicker).
  // cloudsRender() erases old cloud rects with SKY_BLUE and draws the new ones.
  tft.startWrite();

  cloudsRender();     // targeted erase + draw (from the updated clouds.cpp)
  mountainsRender();
  tornadoRender();
  groundRender();     // opaque, drawn last

  tft.endWrite();
}
