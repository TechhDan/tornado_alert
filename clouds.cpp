// clouds.cpp
#include "clouds.h"
#include "colors.h"
#include "config.h"
#include "ground.h"
#include "gfx.h"

namespace {
struct Cloud {
  float x, y;
  float speedPxPerSec;
  int   w, h;
};

constexpr int CLOUD_COUNT         = 3;
constexpr int CLOUD_STRIP_TOP     = 0;   // y on screen where the band starts
constexpr int CLOUD_STRIP_H       = 96;  // band height (fits your layout)
constexpr float CLOUD_RESPAWN_PAD = 20.0f;

Cloud clouds[CLOUD_COUNT] = {
  { 30.0f, 18.0f, 12.0f, 64, 24 },
  {150.0f, 40.0f, 18.0f, 72, 28 },
  {260.0f, 12.0f,  9.0f, 54, 22 },
};

TFT_eSprite cloudCanvas(&tft);
uint16_t cloudColor;

// Draw one fluffy cloud into a sprite (band-local coords)
inline void drawFluffyCloud(TFT_eSprite &dst, int x, int y, int w, int h, uint16_t color) {
  // Clip to band to be safe
  if (y >= CLOUD_STRIP_H || y + h <= 0) return;
  if (x >= SCREEN_W      || x + w <= 0) return;

  int radius = h / 2;
  int baseY  = y + h / 2;

  dst.fillRoundRect(x, y, w, h, radius, color);

  int cxL = x + radius;
  int cxM = x + w / 2;
  int cxR = x + w - radius;

  dst.fillCircle(cxL, baseY - radius / 4, radius,     color);
  dst.fillCircle(cxM, baseY - radius / 2, radius + 2, color);
  dst.fillCircle(cxR, baseY - radius / 5, radius - 1, color);
}
} // namespace

void cloudsInit() {
  cloudColor = tft.color565(250, 250, 255);

  cloudCanvas.setColorDepth(16);
  // If your panel needs it, uncomment:
  // cloudCanvas.setSwapBytes(true);

  if (!cloudCanvas.createSprite(SCREEN_W, CLOUD_STRIP_H)) {
    Serial.println("[clouds] createSprite FAILED (need ~60KB)");
  } else {
    Serial.println("[clouds] createSprite SUCCESS");
  }

  // Pre-fill so first frame has valid background in the band
  cloudCanvas.fillSprite(SKY_BLUE(tft));

  // Optional: draw once so there’s no “pop” on the first animated frame
  for (const auto &c : clouds) {
    drawFluffyCloud(cloudCanvas, (int)c.x, (int)c.y, c.w, c.h, cloudColor);
  }
  // Push the initial band opaquely
  cloudCanvas.pushSprite(0, CLOUD_STRIP_TOP);
}

void cloudsUpdate(uint32_t dt_ms) {
  const float dt = dt_ms / 1000.0f;
  for (auto &c : clouds) {
    c.x += c.speedPxPerSec * dt;
    if (c.x - c.w > SCREEN_W) {
      c.x = -c.w - CLOUD_RESPAWN_PAD;
    }
  }
}

void cloudsRender() {
  // Rebuild the band off-screen each frame, then push once opaquely
  cloudCanvas.fillSprite(SKY_BLUE(tft));

  for (const auto &c : clouds) {
    drawFluffyCloud(cloudCanvas,
                    (int)c.x, (int)c.y,
                    c.w, c.h,
                    cloudColor);
  }

  // IMPORTANT: push OPAQUELY (no transparent color). This overwrites
  // everything in the band at once → no left→right flicker.
  cloudCanvas.pushSprite(0, CLOUD_STRIP_TOP);
}
