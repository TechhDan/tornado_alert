#include "clouds.h"

#include "colors.h"
#include "config.h"
#include "gfx.h"

namespace {
struct Cloud {
  float x;
  float y;
  float speedPxPerSec;
  int   widthPx;
  int   heightPx;
};

constexpr int CLOUD_COUNT         = 3;
constexpr int CLOUD_STRIP_TOP     = 0;
constexpr int CLOUD_STRIP_H       = 96;
constexpr float CLOUD_RESPAWN_PAD = 20.0f;

Cloud clouds[CLOUD_COUNT] = {
    {30.0f, 18.0f, 12.0f, 64, 24},
    {150.0f, 40.0f, 18.0f, 72, 28},
    {260.0f, 12.0f, 9.0f, 54, 22},
};

TFT_eSprite cloudCanvas(&tft);

uint16_t cloudColor;

void drawFluffyCloud(TFT_eSprite &canvas, int x, int y, int w, int h, uint16_t color) {
  int radius = h / 2;
  int baseY  = y + h / 2;

  canvas.fillRoundRect(x, y, w, h, radius, color);

  int leftBubbleCenterX   = x + radius;
  int middleBubbleCenterX = x + w / 2;
  int rightBubbleCenterX  = x + w - radius;

  canvas.fillCircle(leftBubbleCenterX, baseY - radius / 4, radius, color);
  canvas.fillCircle(middleBubbleCenterX, baseY - radius / 2, radius + 2, color);
  canvas.fillCircle(rightBubbleCenterX, baseY - radius / 5, radius - 1, color);
}
} // namespace

void cloudsInit() {
  cloudColor = tft.color565(250, 250, 255);

  cloudCanvas.setColorDepth(16);
  cloudCanvas.createSprite(SCREEN_W, CLOUD_STRIP_H);

}

void cloudsUpdate(uint32_t dt_ms) {
  float dtSeconds = dt_ms / 1000.0f;
  for (auto &cloud : clouds) {
    cloud.x += cloud.speedPxPerSec * dtSeconds;
    if (cloud.x - cloud.widthPx > SCREEN_W) {
      cloud.x = -cloud.widthPx - CLOUD_RESPAWN_PAD;
    }
  }
}

void cloudsRender() {
  cloudCanvas.fillSprite(SKY_BLUE(tft));

  for (const auto &cloud : clouds) {
    int drawX = static_cast<int>(cloud.x);
    int drawY = static_cast<int>(cloud.y);
    if (drawX > SCREEN_W || drawX + cloud.widthPx < 0) {
      continue;
    }
    drawFluffyCloud(cloudCanvas, drawX, drawY, cloud.widthPx, cloud.heightPx, cloudColor);
  }

  cloudCanvas.pushSprite(0, CLOUD_STRIP_TOP);
}
