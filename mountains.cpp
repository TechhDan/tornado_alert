#include "mountains.h"

#include "gfx.h"
#include "ground.h"
#include "colors.h"
#include "config.h"

namespace {
struct MountainLayer {
  float scrollPx;
  int   speedPxPerSec;
  int   heightPx;
  int   patternWidth;
  uint16_t baseColor;
  uint16_t highlightColor;
};

constexpr int SCROLL_DIR = +1; // +1 = left->right, -1 = right->left
constexpr int LAYER_COUNT = 2;

MountainLayer layers[LAYER_COUNT] = {
  {0.0f, 12, 36, 80,  0, 0},
  {0.0f, 20, 54, 110, 0, 0}
};

// height in pixels that the mountain strip occupies. We use the tallest layer.
constexpr int STRIP_HEIGHT = 64;

inline uint16_t makeColor(uint8_t r, uint8_t g, uint8_t b) {
  return tft.color565(r, g, b);
}
} // namespace

void mountainsInit() {
  // Replace default colors with runtime computed ones to ensure tft initialized.
  layers[0].baseColor      = makeColor(80, 120, 160);
  layers[0].highlightColor = makeColor(180, 210, 230);
  layers[1].baseColor      = makeColor(60, 100, 140);
  layers[1].highlightColor = makeColor(160, 190, 210);

  for (auto &layer : layers) {
    layer.scrollPx = 0.0f;
  }
}

void mountainsUpdate(uint32_t dt_ms) {
  float dtSeconds = dt_ms / 1000.0f;
  for (auto &layer : layers) {
    float delta = layer.speedPxPerSec * dtSeconds * SCROLL_DIR;
    layer.scrollPx += delta;
    while (layer.scrollPx >= layer.patternWidth) {
      layer.scrollPx -= layer.patternWidth;
    }
    while (layer.scrollPx < 0.0f) {
      layer.scrollPx += layer.patternWidth;
    }
  }
}

void mountainsRender() {
  const int baseY = SCREEN_H - GROUND_HEIGHT; // align with top of ground
  const int topY  = baseY - STRIP_HEIGHT;

  tft.fillRect(0, topY, SCREEN_W, STRIP_HEIGHT, SKY_BLUE(tft));

  for (int i = 0; i < LAYER_COUNT; ++i) {
    MountainLayer &layer = layers[i];
    int startX = -static_cast<int>(layer.scrollPx);
    int cols   = (SCREEN_W / layer.patternWidth) + 3;

    for (int c = 0; c < cols; ++c) {
      int xLeft  = startX + c * layer.patternWidth;
      int xRight = xLeft + layer.patternWidth;
      int peakX  = xLeft + (layer.patternWidth / 2);
      int peakY  = baseY - layer.heightPx;

      tft.fillTriangle(xLeft, baseY, xRight, baseY, peakX, peakY, layer.baseColor);

      int highlightLeft   = peakX - layer.patternWidth / 8;
      int highlightRight  = peakX + layer.patternWidth / 8;
      int highlightBaseY  = baseY - (layer.heightPx / 3);
      int highlightPeakY  = peakY;
      tft.fillTriangle(highlightLeft, highlightBaseY, highlightRight, highlightBaseY, peakX,
                       highlightPeakY, layer.highlightColor);
    }
  }
}
