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

static TFT_eSprite mountainsCanvas(&tft);

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

  mountainsCanvas.setColorDepth(16);
  mountainsCanvas.createSprite(SCREEN_W, STRIP_HEIGHT);
  // Optional (some drivers prefer this when using 16-bit sprites):
  // mountainsCanvas.setSwapBytes(true);

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

void mountainsRender() {
  // Align strip exactly above the ground (no +1 to avoid a gap or overdraw)
  const int topY = (SCREEN_H - GROUND_HEIGHT) - STRIP_HEIGHT;

  // Draw into the off-screen band. Fill with the real sky color first.
  mountainsCanvas.fillSprite(SKY_BLUE(tft));

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
      mountainsCanvas.fillTriangle(xLeft, baseRow, xRight, baseRow, peakX, peakY,
                                   layer.baseColor);

      // Highlight wedge
      int highlightLeft   = peakX - layer.patternWidth / 8;
      int highlightRight  = peakX + layer.patternWidth / 8;
      int highlightBaseY  = baseRow - (layer.heightPx / 3);
      int highlightPeakY  = peakY;
      if (highlightBaseY < highlightPeakY) highlightBaseY = highlightPeakY;

      mountainsCanvas.fillTriangle(highlightLeft, highlightBaseY,
                                   highlightRight, highlightBaseY,
                                   peakX, highlightPeakY,
                                   layer.highlightColor);
    }
  }

  // IMPORTANT: push OPAQUELY (no transparent color). This overwrites
  // old pixels (including newly revealed sky) in one go → no trails/flicker.
  mountainsCanvas.pushSprite(0, topY);
}
