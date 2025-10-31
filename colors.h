#pragma once
#include <TFT_eSPI.h>

inline uint16_t SKY_BLUE(TFT_eSPI& tft){ return tft.color565(135,206,235); }
inline uint16_t C_CLEAR (TFT_eSPI& tft){ return tft.color565(255,0,255); }
inline uint16_t C_BODY  (TFT_eSPI& tft){ return tft.color565(255,220,40); }
inline uint16_t C_SHADOW(TFT_eSPI& tft){ return tft.color565(200,150,0); }
