#include "town.h"

#include <algorithm>

#include "gfx.h"
#include "config.h"
#include "ground.h"

namespace {
struct Town {
  float x;
  int   variant;
  int   width;
};

constexpr int   TOWN_COUNT          = 3;
constexpr float TOWN_SPEED_PX_PER_S = 26.0f;
constexpr int   MIN_GAP_PX          = 70;
constexpr int   MAX_GAP_PX          = 120;

Town towns[TOWN_COUNT];

uint16_t houseWallLight;
uint16_t houseWallDark;
uint16_t roofRed;
uint16_t roofGray;
uint16_t windowGlow;
uint16_t barnRed;
uint16_t siloMetal;
uint16_t treeLeaf;
uint16_t treeTrunk;

int groundLine() {
  return (SCREEN_H - GROUND_HEIGHT) - 1;  // sprite coordinates (0 at top of sky band)
}

void drawHouse(TFT_eSprite &dst, int x, int baseY, int width, int height,
               uint16_t wallColor, uint16_t roofColor, bool doorCenter) {
  int topY = baseY - height + 1;
  if (topY >= baseY) return;
  dst.fillRect(x, topY, width, height, wallColor);

  int roofHeight = std::max(4, height / 3);
  int roofPeakX  = x + width / 2;
  dst.fillTriangle(x - 1, topY + roofHeight, x + width + 1, topY + roofHeight,
                   roofPeakX, topY - roofHeight / 2, roofColor);

  int windowW = std::max(4, width / 5);
  int windowH = std::max(4, height / 4);
  int windowY = topY + height / 3;
  dst.fillRect(x + width / 5, windowY, windowW, windowH, windowGlow);
  dst.fillRect(x + width - windowW - width / 5, windowY, windowW, windowH, windowGlow);

  int doorW = std::max(4, width / 4);
  int doorH = height / 3;
  int doorX = doorCenter ? (roofPeakX - doorW / 2) : (x + width / 6);
  int doorY = baseY - doorH + 1;
  dst.fillRect(doorX, doorY, doorW, doorH, roofColor);
}

void drawBarn(TFT_eSprite &dst, int x, int baseY, int width, int height) {
  int topY = baseY - height + 1;
  dst.fillRect(x, topY, width, height, barnRed);
  int roofHeight = height / 2;
  dst.fillTriangle(x, topY + roofHeight, x + width, topY + roofHeight,
                   x + width / 2, topY - roofHeight / 2, roofGray);
  int doorW = width / 3;
  int doorH = height / 2;
  int doorX = x + (width - doorW) / 2;
  int doorY = baseY - doorH + 1;
  dst.fillRect(doorX, doorY, doorW, doorH, roofGray);
  dst.drawFastVLine(doorX + doorW / 2, doorY, doorH, barnRed);
  dst.drawFastHLine(doorX, doorY + doorH / 2, doorW, barnRed);
}

void drawSilo(TFT_eSprite &dst, int x, int baseY, int width, int height) {
  int topY = baseY - height + 1;
  dst.fillRect(x, topY, width, height, siloMetal);
  dst.fillTriangle(x - 1, topY + width / 3, x + width + 1, topY + width / 3,
                   x + width / 2, topY - width / 3, roofGray);
}

void drawTree(TFT_eSprite &dst, int x, int baseY, int height) {
  int trunkH = height / 3;
  int trunkW = std::max(3, height / 6);
  dst.fillRect(x, baseY - trunkH + 1, trunkW, trunkH, treeTrunk);
  int crownR = height / 2;
  dst.fillCircle(x + trunkW / 2, baseY - trunkH - crownR / 2, crownR, treeLeaf);
}

int townWidthForVariant(int variant) {
  switch (variant % 3) {
    case 0: return 68;
    case 1: return 84;
    default: return 76;
  }
}

void drawTownVariant(TFT_eSprite &dst, int variant, int originX) {
  int baseY = groundLine();
  switch (variant % 3) {
    case 0: {
      drawHouse(dst, originX, baseY, 26, 34, houseWallLight, roofRed, true);
      drawHouse(dst, originX + 28, baseY, 24, 28, houseWallDark, roofGray, false);
      drawTree(dst, originX + 56, baseY, 26);
      break;
    }
    case 1: {
      drawBarn(dst, originX, baseY, 36, 40);
      drawSilo(dst, originX + 38, baseY, 14, 44);
      drawTree(dst, originX + 56, baseY, 30);
      drawHouse(dst, originX + 66, baseY, 18, 26, houseWallLight, roofRed, false);
      break;
    }
    case 2: {
      drawHouse(dst, originX, baseY, 22, 28, houseWallDark, roofRed, false);
      drawHouse(dst, originX + 24, baseY, 20, 32, houseWallLight, roofGray, true);
      drawHouse(dst, originX + 46, baseY, 22, 26, houseWallDark, roofRed, true);
      drawTree(dst, originX + 68, baseY, 24);
      break;
    }
  }
}

int randomGapForVariant(int variant) {
  // simple pseudo variation without RNG
  switch (variant % 3) {
    case 0: return MIN_GAP_PX;
    case 1: return (MIN_GAP_PX + MAX_GAP_PX) / 2;
    default: return MAX_GAP_PX;
  }
}

}  // namespace

void townInit() {
  houseWallLight = tft.color565(235, 220, 200);
  houseWallDark  = tft.color565(200, 180, 160);
  roofRed        = tft.color565(180, 60, 60);
  roofGray       = tft.color565(120, 120, 130);
  windowGlow     = tft.color565(250, 240, 180);
  barnRed        = tft.color565(170, 40, 40);
  siloMetal      = tft.color565(180, 180, 190);
  treeLeaf       = tft.color565(40, 140, 70);
  treeTrunk      = tft.color565(110, 70, 40);

  float cursor = SCREEN_W + 40.0f;
  for (int i = 0; i < TOWN_COUNT; ++i) {
    towns[i].variant = i % 3;
    towns[i].width   = townWidthForVariant(towns[i].variant);
    towns[i].x       = cursor;
    cursor += towns[i].width + randomGapForVariant(towns[i].variant);
  }
}

void townUpdate(uint32_t dt_ms) {
  float dt = dt_ms / 1000.0f;
  for (int i = 0; i < TOWN_COUNT; ++i) {
    Town &town = towns[i];
    town.x -= TOWN_SPEED_PX_PER_S * dt;
  }

  // recycle towns that moved off-screen
  for (int i = 0; i < TOWN_COUNT; ++i) {
    Town &town = towns[i];
    if (town.x + town.width < 0) {
      // find farthest town edge to place after it
      float farthestEdge = town.x + town.width;
      for (int j = 0; j < TOWN_COUNT; ++j) {
        float rightEdge = towns[j].x + towns[j].width;
        if (rightEdge > farthestEdge) {
          farthestEdge = rightEdge;
        }
      }
      town.variant = (town.variant + 1) % 3;
      town.width   = townWidthForVariant(town.variant);
      town.x       = farthestEdge + randomGapForVariant(town.variant);
    }
  }
}

void townRender(TFT_eSprite &dst) {
  for (const Town &town : towns) {
    int x = static_cast<int>(town.x);
    if (x >= SCREEN_W || x + town.width <= 0) continue;
    drawTownVariant(dst, town.variant, x);
  }
}
