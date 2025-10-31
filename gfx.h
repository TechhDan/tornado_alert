#pragma once
#include <TFT_eSPI.h>

extern TFT_eSPI tft;
extern TFT_eSprite spr;

void gfxInit();
void gfxClearSky();
void spriteBuild();
void spriteErase(int x, int y, int w, int h);
void spriteDraw(int x, int y, uint16_t transparentKey);
