#pragma once
#include <stdint.h>

class TFT_eSprite;

void tornadoInit();
void tornadoUpdate(uint32_t dt_ms);
void tornadoRender(TFT_eSprite &dst);
float tornadoWorldX();

