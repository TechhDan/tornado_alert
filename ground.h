#pragma once
#include <stdint.h>

void groundInit();
void groundUpdate(uint32_t dt_ms);  // pass elapsed ms this frame
void groundRender();
