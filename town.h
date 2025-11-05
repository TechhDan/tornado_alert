#ifndef TOWN_H
#define TOWN_H

#include <Arduino.h>

class TFT_eSprite;

void townInit();
void townUpdate(uint32_t dt_ms);
void townRender(TFT_eSprite &dst);
void townSetAlert(bool active);

#endif // TOWN_H
