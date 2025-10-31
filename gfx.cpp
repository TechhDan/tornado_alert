#include "gfx.h"
#include "config.h"
#include "colors.h"

TFT_eSPI tft = TFT_eSPI();

void gfxInit() {
  tft.init();
  tft.setRotation(1);            // 320x240 landscape
}

void gfxClearSky() {
  tft.fillScreen(SKY_BLUE(tft));
}
