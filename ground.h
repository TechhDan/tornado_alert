#pragma once
#include <stdint.h>

constexpr int GROUND_HEIGHT = 48;  // pixels occupied by the ground strip

void groundInit();
void groundUpdate(uint32_t dt_ms);  // pass elapsed ms this frame
void groundRender();
