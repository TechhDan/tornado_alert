#include "mountains.h"

#include "gfx.h"
#include "ground.h"
#include "colors.h"
#include "config.h"

namespace {
struct MountainLayer {
  float     scrollPx;
  int       speedPxPerSec;
  int       heightPx;
  int       patternWidth;
  uint16_t  baseColor;
  uint16_t  highlightColor;
};

constexpr int SCROLL_DIR  = +1;   // +1 = left->right, -1 = right->left
constexpr int LAYER_COUNT = 2;

// layer params: scroll, speed, height, pattern width, colors filled at init
MountainLayer layers[LAYER_COUNT] = {
  {0.0f, 12, 36,  80, 0, 0},
  {0.0f, 20, 54, 110, 0, 0}
};

// Tallest vertical span of mountains (px)
constexpr int STRIP_HEIGHT = 64;

inline uint16_t makeColor(uint8_t r, uint8_t g, uint8_t b) {
  return tft.color565(r, g, b);
}
} // namespace

void mountainsInit() {
  // Colors set after tft.init()
  layers[0].baseColor      = makeColor(80, 120, 160);
  layers[0].highlightColor = makeColor(180, 210, 230);
  layers[1].baseColor      = makeColor(60, 100, 140);
  layers[1].highlightColor = makeColor(160, 190, 210);

  for (auto &layer : layers) layer.scrollPx = 0.0f;
}

void mountainsUpdate(uint32_t dt_ms) {
  const float dtSeconds = dt_ms / 1000.0f;
  for (auto &layer : layers) {
    const float delta = layer.speedPxPerSec * dtSeconds * SCROLL_DIR;
    layer.scrollPx += delta;
    while (layer.scrollPx >= layer.patternWidth) layer.scrollPx -= layer.patternWidth;
    while (layer.scrollPx < 0.0f)               layer.scrollPx += layer.patternWidth;
  }
}

void mountainsRender(TFT_eSprite &dst) {
  // Align strip exactly above the ground (no +1 to avoid a gap or overdraw)
  const int topY = (SCREEN_H - GROUND_HEIGHT) - STRIP_HEIGHT;

  for (int i = 0; i < LAYER_COUNT; ++i) {
    MountainLayer &layer = layers[i];

    const int startX = -static_cast<int>(layer.scrollPx);
    const int cols   = (SCREEN_W / layer.patternWidth) + 3;

    for (int c = 0; c < cols; ++c) {
      const int xLeft  = startX + c * layer.patternWidth;
      const int xRight = xLeft + layer.patternWidth;
      if (xRight <= 0 || xLeft >= SCREEN_W) continue;

      const int peakX   = xLeft + (layer.patternWidth / 2);
      const int peakY   = STRIP_HEIGHT - 1 - layer.heightPx;
      const int baseRow = STRIP_HEIGHT - 1;

      // Main mountain
      dst.fillTriangle(xLeft, topY + baseRow, xRight, topY + baseRow, peakX, topY + peakY,
                       layer.baseColor);

      // Highlight wedge
      int highlightLeft   = peakX - layer.patternWidth / 8;
      int highlightRight  = peakX + layer.patternWidth / 8;
      int highlightBaseY  = baseRow - (layer.heightPx / 3);
      int highlightPeakY  = peakY;
      if (highlightBaseY < highlightPeakY) highlightBaseY = highlightPeakY;

      dst.fillTriangle(highlightLeft, topY + highlightBaseY,
                       highlightRight, topY + highlightBaseY,
                       peakX, topY + highlightPeakY,
                       layer.highlightColor);
    }
  }
}
