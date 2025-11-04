#include "tornado.h"

#include <math.h>

#include "gfx.h"
#include "ground.h"
#include "colors.h"
#include "config.h"

namespace {
constexpr int SPRITE_W      = 120;
constexpr int SPRITE_H      = 240 - GROUND_HEIGHT;
constexpr int BAND_HEIGHT   = 4;
constexpr int TOP_WIDTH     = 72;
constexpr int BOTTOM_WIDTH  = 20;
constexpr float SWIRL_FREQ  = 6.0f;
constexpr float SPIN_SPEED  = 2.2f;      // radians per second
constexpr float SWAY_AMOUNT = 8.0f;      // horizontal wobble inside sprite
constexpr int POSITION_X    = 24;        // screen X for sprite
constexpr float TAU         = 6.28318530718f;
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
  baseColor   = makeColor(160, 160, 176);
  darkColor   = makeColor(120, 120, 140);
  lightColor  = makeColor(210, 210, 220);
  debrisColor = makeColor(120,  90,  60);

  spinPhase = 0.0f;
}

void tornadoUpdate(uint32_t dt_ms) {
  float dt = dt_ms / 1000.0f;
  spinPhase += SPIN_SPEED * dt;
  if (spinPhase > TAU) spinPhase -= TAU;
}

static void drawFunnel(TFT_eSprite &dst, int originX, int originY) {
  const float centerX = SPRITE_W / 2.0f;

  // Funnel bands
  for (int y = 0; y < SPRITE_H; y += BAND_HEIGHT) {
    float t = static_cast<float>(y) / (SPRITE_H - 1);
    float width = TOP_WIDTH + (BOTTOM_WIDTH - TOP_WIDTH) * t;

    float swirl = sinf(spinPhase + t * SWIRL_FREQ) * SWAY_AMOUNT;
    int left = static_cast<int>(centerX - width / 2.0f + swirl);
    int bandWidth = static_cast<int>(width);
    if (bandWidth <= 0) continue;

    // Clamp horizontally so we never draw outside the sprite
    if (left < 0) { bandWidth += left; left = 0; }
    if (left + bandWidth > SPRITE_W) bandWidth = SPRITE_W - left;
    if (bandWidth <= 0) continue;

    float shadeLerp = (sinf(spinPhase * 2.0f + t * 10.0f) + 1.0f) * 0.5f;
    uint16_t fillColor = (shadeLerp > 0.66f) ? lightColor
                         : (shadeLerp < 0.33f) ? darkColor
                         : baseColor;

    dst.fillRect(originX + left, originY + y, bandWidth, BAND_HEIGHT, fillColor);

    // Subtle highlight stripe
    int highlightWidth = bandWidth / 3;
    if (highlightWidth > 0) {
      int highlightX = left + bandWidth / 2
                     + static_cast<int>(sinf(spinPhase + t * 8.0f) * (bandWidth * 0.15f));
      highlightX -= highlightWidth / 2;
      // Clamp highlight too
      if (highlightX < 0) { highlightWidth += highlightX; highlightX = 0; }
      if (highlightX + highlightWidth > SPRITE_W) highlightWidth = SPRITE_W - highlightX;
      if (highlightWidth > 0)
        dst.fillRect(originX + highlightX, originY + y, highlightWidth, BAND_HEIGHT, lightColor);
    }
  }

  // Swirling base shadow
  int baseY = SPRITE_H - BAND_HEIGHT;
  for (int i = 0; i < 3; ++i) {
    int radius = 26 + i * 6;
    int offset = static_cast<int>(sinf(spinPhase * 1.5f + i) * 6.0f);
    int cx = static_cast<int>(centerX + offset);
    // Clamp ellipse center horizontally (avoid drawing outside sprite)
    if (cx - radius < 0) cx = radius;
    if (cx + radius >= SPRITE_W) cx = SPRITE_W - 1 - radius;
    dst.fillEllipse(originX + cx, originY + baseY + BAND_HEIGHT, radius, BAND_HEIGHT, darkColor);
  }

  // Debris specks
  for (int i = 0; i < 12; ++i) {
    float angle = spinPhase * 2.0f + i;
    int debrisX = static_cast<int>(centerX + cosf(angle) * (BOTTOM_WIDTH / 2.0f + 8));
    int debrisY = baseY - (i % 3);
    if (debrisX >= 0 && debrisX < SPRITE_W && debrisY >= 0 && debrisY < SPRITE_H)
      dst.drawPixel(originX + debrisX, originY + debrisY, debrisColor);
  }
}

void tornadoRender(TFT_eSprite &dst) {
  const int baseY = (SCREEN_H - GROUND_HEIGHT) - SPRITE_H;
  drawFunnel(dst, POSITION_X, baseY);
}
