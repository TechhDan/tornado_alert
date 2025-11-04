#pragma once

#include <Arduino.h>

class TFT_eSprite;

void cloudsInit();
void cloudsUpdate(uint32_t dt_ms);
void cloudsRender(TFT_eSprite &dst);
