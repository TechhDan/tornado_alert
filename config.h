#pragma once

// Screen (landscape 320x240)
constexpr int SCREEN_W = 320;
constexpr int SCREEN_H = 240;

// Frame timing
constexpr uint32_t FRAME_MS = 16;  // ~60 FPS

// SPI pins (match your wiring)
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_DC    2
#define TFT_RST   4
#define TFT_CS    5
#define TFT_MISO 19  // (unused by LCD, harmless)

// Gameplay inputs
constexpr int ALERT_BUTTON_PIN = 25;      // GPIO25 has an internal pull-up
constexpr uint32_t ALERT_DURATION_MS = 8000;  // how long citizens stay sheltered
