#ifndef MOUNTAINS_H
#define MOUNTAINS_H

#include <Arduino.h>

class TFT_eSprite;

void mountainsInit();
void mountainsUpdate(uint32_t dt_ms);
void mountainsRender(TFT_eSprite &dst);

#endif // MOUNTAINS_H
