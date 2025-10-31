#include "tornado.h"

#include <math.h>

#include "gfx.h"
#include "ground.h"
#include "colors.h"
#include "config.h"

namespace {
constexpr int SPRITE_W      = 120;
constexpr int SPRITE_H      = 120;
constexpr int BAND_HEIGHT   = 4;
constexpr int TOP_WIDTH     = 20;
constexpr int BOTTOM_WIDTH  = 72;
constexpr float SWIRL_FREQ  = 6.0f;
constexpr float SPIN_SPEED  = 2.2f; // radians per second
constexpr float SWAY_AMOUNT = 8.0f;
constexpr int POSITION_X    = 24;   // screen position for sprite (left side)
constexpr float TWO_PI      = 6.28318530718f;

TFT_eSprite tornadoSprite(&tft);
float spinPhase = 0.0f;

inline uint16_t makeColor(uint8_t r, uint8_t g, uint8_t b) {
  return tft.color565(r, g, b);
}

uint16_t baseColor;
uint16_t darkColor;
uint16_t lightColor;
uint16_t debrisColor;
}  // namespace

void tornadoInit() {
  tornadoSprite.setColorDepth(16);
  tornadoSprite.createSprite(SPRITE_W, SPRITE_H);

  baseColor   = makeColor(160, 160, 176);
  darkColor   = makeColor(120, 120, 140);
  lightColor  = makeColor(210, 210, 220);
  debrisColor = makeColor(120, 90, 60);

  spinPhase = 0.0f;
}

void tornadoUpdate(uint32_t dt_ms) {
  float dt = dt_ms / 1000.0f;
  spinPhase += SPIN_SPEED * dt;
  if (spinPhase > TWO_PI) {
    spinPhase -= TWO_PI;
  }
}

static void drawFunnel() {
  tornadoSprite.fillSprite(SKY_BLUE(tft));

  const float centerX = SPRITE_W / 2.0f;

  for (int y = 0; y < SPRITE_H; y += BAND_HEIGHT) {
    float t = static_cast<float>(y) / (SPRITE_H - 1);
    float width = TOP_WIDTH + (BOTTOM_WIDTH - TOP_WIDTH) * t;

    float swirl = sinf(spinPhase + t * SWIRL_FREQ) * SWAY_AMOUNT;
    int left = static_cast<int>(centerX - width / 2.0f + swirl);
    int bandWidth = static_cast<int>(width);

    if (bandWidth <= 0) {
      continue;
    }

    float shadeLerp = (sinf(spinPhase * 2.0f + t * 10.0f) + 1.0f) * 0.5f;
    uint16_t fillColor;
    if (shadeLerp > 0.66f) {
      fillColor = lightColor;
    } else if (shadeLerp < 0.33f) {
      fillColor = darkColor;
    } else {
      fillColor = baseColor;
    }

    tornadoSprite.fillRect(left, y, bandWidth, BAND_HEIGHT, fillColor);

    // Draw a subtle highlight stripe offset toward spin direction
    int highlightWidth = bandWidth / 3;
    if (highlightWidth > 0) {
      int highlightX = left + bandWidth / 2 + static_cast<int>(sinf(spinPhase + t * 8.0f) * (bandWidth * 0.15f));
      highlightX -= highlightWidth / 2;
      tornadoSprite.fillRect(highlightX, y, highlightWidth, BAND_HEIGHT, lightColor);
    }
  }

  // Add a swirling base shadow to tie into the ground
  int baseY = SPRITE_H - BAND_HEIGHT;
  for (int i = 0; i < 3; ++i) {
    int radius = 26 + i * 6;
    int offset = static_cast<int>(sinf(spinPhase * 1.5f + i) * 6.0f);
    int x = static_cast<int>(centerX + offset) - radius;
    tornadoSprite.fillEllipse(x + radius, baseY + BAND_HEIGHT, radius, BAND_HEIGHT, darkColor);
  }

  // Little debris specks whipping around the base
  for (int i = 0; i < 12; ++i) {
    float angle = spinPhase * 2.0f + i;
    int debrisX = static_cast<int>(centerX + cosf(angle) * (BOTTOM_WIDTH / 2.0f + 8));
    int debrisY = baseY - (i % 3);
    tornadoSprite.drawPixel(debrisX, debrisY, debrisColor);
  }
}

void tornadoRender() {
  drawFunnel();

  int baseScreenY = SCREEN_H - GROUND_HEIGHT - SPRITE_H;
  if (baseScreenY < 0) {
    baseScreenY = 0;
  }
  tornadoSprite.pushSprite(POSITION_X, baseScreenY);
}
