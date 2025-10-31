#include "gfx.h"
#include "config.h"
#include "colors.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

void gfxInit() {
  tft.init();
  tft.setRotation(1);            // 320x240 landscape
}

void gfxClearSky() {
  tft.fillScreen(SKY_BLUE(tft));
}

void spriteBuild() {
  spr.setColorDepth(16);
  spr.createSprite(16, 16);
  spr.fillSprite(C_CLEAR(tft));

  // simple “blob”
  spr.fillCircle(8, 8, 7, C_BODY(tft));
  spr.fillCircle(10,10, 6, C_SHADOW(tft));
  spr.fillCircle(7, 7, 6, C_BODY(tft));
  spr.fillCircle(6, 6, 2, TFT_WHITE);
  spr.fillCircle(10,7, 2, TFT_WHITE);
  spr.fillCircle(6, 6, 1, TFT_BLACK);
  spr.fillCircle(10,7, 1, TFT_BLACK);
}

void spriteErase(int x, int y, int w, int h) {
  tft.fillRect(x, y, w, h, SKY_BLUE(tft));
}

void spriteDraw(int x, int y, uint16_t transparentKey) {
  spr.pushSprite(x, y, transparentKey);
}
