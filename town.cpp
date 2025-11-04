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
  const int houseTop  = baseY - height + 1;
  const int roofH     = std::max(4, height / 3);
  const int wallTop   = houseTop + roofH;           // start wall under the eaves
  const int wallH     = height - roofH;
  if (wallH <= 0) return;

  // wall (clipped)
  dst.fillRect(x, wallTop, width, wallH, wallColor);

  // roof
  const int peakX = x + width / 2;
  dst.fillTriangle(x - 1, wallTop, x + width + 1, wallTop,
                   peakX, houseTop - roofH / 2, roofColor);

  // windows (relative to wall)
  const int windowW = std::max(4, width / 5);
  const int windowH = std::max(4, wallH / 4);
  const int windowY = wallTop + wallH / 3;
  dst.fillRect(x + width / 5, windowY, windowW, windowH, windowGlow);
  dst.fillRect(x + width - windowW - width / 5, windowY, windowW, windowH, windowGlow);

  // door
  const int doorW = std::max(4, width / 4);
  const int doorH = wallH / 2;                      // looks nicer with clipped wall
  const int doorX = doorCenter ? (peakX - doorW / 2) : (x + width / 6);
  const int doorY = baseY - doorH + 1;
  dst.fillRect(doorX, doorY, doorW, doorH, roofColor);
}

void drawBarn(TFT_eSprite &dst, int x, int baseY, int width, int height) {
  const int topY   = baseY - height + 1;
  const int roofH  = height / 2;
  const int wallTop= topY + roofH;
  const int wallH  = height - roofH;

  dst.fillRect(x, wallTop, width, wallH, barnRed);
  dst.fillTriangle(x, wallTop, x + width, wallTop,
                   x + width / 2, topY - roofH / 2, roofGray);

  const int doorW = width / 3, doorH = wallH / 2;
  const int doorX = x + (width - doorW) / 2;
  const int doorY = baseY - doorH + 1;
  dst.fillRect(doorX, doorY, doorW, doorH, roofGray);
  dst.drawFastVLine(doorX + doorW / 2, doorY, doorH, barnRed);
  dst.drawFastHLine(doorX, doorY + doorH / 2, doorW, barnRed);
}

void drawSilo(TFT_eSprite &dst, int x, int baseY, int width, int height) {
  const int topY     = baseY - height + 1;
  const int roofBase = topY + width / 3;            // your cone base line
  const int wallH    = baseY - roofBase + 1;

  dst.fillRect(x, roofBase, width, wallH, siloMetal);
  dst.fillTriangle(x - 1, roofBase, x + width + 1, roofBase,
                   x + width / 2, topY - width / 3, roofGray);
}

void drawTree(TFT_eSprite &dst, int x, int baseY, int height) {
  // Adjust proportions for better visibility
  int trunkH = height / 2;                       // taller trunk (was /3)
  int trunkW = std::max(4, height / 5);          // slightly wider trunk

  // Draw trunk first
  int trunkX = x + height / 2 - trunkW / 2;      // center trunk under crown
  dst.fillRect(trunkX, baseY - trunkH + 1, trunkW, trunkH, treeTrunk);

  // Draw crown slightly higher
  int crownR = height / 2;
  int crownY = baseY - trunkH - crownR / 3;      // raise crown for visibility
  dst.fillCircle(x + height / 2, crownY, crownR, treeLeaf);
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
