#include "town.h"

#include <algorithm>
#include <math.h>

#include "gfx.h"
#include "config.h"
#include "ground.h"

namespace {
struct Citizen {
  float    offsetX;
  float    bobPhase;
  float    bobSpeed;
  uint16_t shirtColor;
  uint16_t skinColor;
};

constexpr int   MAX_CITIZENS_PER_TOWN = 4;

struct Town {
  float    x;
  int      variant;
  int      width;
  int      citizenCount;
  Citizen  citizens[MAX_CITIZENS_PER_TOWN];
};

constexpr int   TOWN_COUNT            = 3;
constexpr float TOWN_SPEED_PX_PER_S   = 26.0f;
constexpr int   MIN_GAP_PX            = 70;
constexpr int   MAX_GAP_PX            = 120;
constexpr float TAU_F               = 6.28318530718f;
constexpr float CITIZEN_BOB_AMPLITUDE = 2.5f;

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
uint16_t citizenPants;
uint16_t citizenHair;
uint16_t citizenShadow;
uint16_t citizenSkinLight;
uint16_t citizenSkinDark;
uint16_t citizenShirtPalette[5];

struct CitizenLayout {
  int   count;
  float offsets[MAX_CITIZENS_PER_TOWN];
};

constexpr CitizenLayout citizenLayouts[] = {
    {3, {12.0f, 34.0f, 54.0f, 0.0f}},  // variant 0
    {4, {10.0f, 32.0f, 56.0f, 72.0f}}, // variant 1
    {3, { 8.0f, 30.0f, 52.0f, 0.0f}},  // variant 2
};

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

void initTownCitizens(Town &town, int townIndex) {
  const CitizenLayout &layout = citizenLayouts[town.variant % 3];
  town.citizenCount = layout.count;
  for (int i = 0; i < layout.count; ++i) {
    Citizen &cit = town.citizens[i];
    float desiredOffset = layout.offsets[i];
    float maxOffset     = static_cast<float>(std::max(4, town.width - 4));
    cit.offsetX         = std::min(desiredOffset, maxOffset);
    cit.offsetX         = std::max(4.0f, cit.offsetX);
    cit.bobPhase  = (townIndex + 1) * 0.9f + i * 1.1f;
    cit.bobSpeed  = 1.2f + 0.25f * ((town.variant + i) % 4);
    cit.shirtColor = citizenShirtPalette[(town.variant + townIndex + i) % 5];
    cit.skinColor  = ((townIndex + i) % 2 == 0) ? citizenSkinLight : citizenSkinDark;
  }
}

void updateTownCitizens(Town &town, float dt) {
  for (int i = 0; i < town.citizenCount; ++i) {
    Citizen &cit = town.citizens[i];
    cit.bobPhase += cit.bobSpeed * dt;
    if (cit.bobPhase > TAU_F) {
      cit.bobPhase = fmodf(cit.bobPhase, TAU_F);
    }
  }
}

void drawCitizen(TFT_eSprite &dst, int x, int baseY, const Citizen &cit) {
  int bobOffset = static_cast<int>(sinf(cit.bobPhase) * CITIZEN_BOB_AMPLITUDE);
  int footY     = baseY - bobOffset;

  // ground shadow
  dst.drawFastHLine(x - 3, baseY + 1, 6, citizenShadow);

  // legs
  dst.fillRect(x - 2, footY - 5, 2, 5, citizenPants);
  dst.fillRect(x,     footY - 5, 2, 5, citizenPants);

  // torso
  int torsoY = footY - 10;
  dst.fillRect(x - 3, torsoY, 6, 6, cit.shirtColor);

  // arms as small dots on the side of torso
  dst.drawPixel(x - 4, torsoY + 2, cit.skinColor);
  dst.drawPixel(x + 3, torsoY + 2, cit.skinColor);

  // head and hair
  dst.fillCircle(x, torsoY - 2, 2, cit.skinColor);
  dst.drawFastHLine(x - 2, torsoY - 4, 4, citizenHair);
  dst.drawPixel(x - 2, torsoY - 3, citizenHair);
  dst.drawPixel(x + 1, torsoY - 3, citizenHair);
}

void drawTownCitizens(TFT_eSprite &dst, const Town &town, int originX) {
  int baseY = groundLine();
  for (int i = 0; i < town.citizenCount; ++i) {
    const Citizen &cit = town.citizens[i];
    int screenX = originX + static_cast<int>(cit.offsetX);
    if (screenX + 4 < 0 || screenX - 4 >= SCREEN_W) continue;
    drawCitizen(dst, screenX, baseY, cit);
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
  citizenPants   = tft.color565(40, 70, 120);
  citizenHair    = tft.color565(60, 40, 30);
  citizenShadow  = tft.color565(70, 70, 80);
  citizenSkinLight = tft.color565(255, 224, 189);
  citizenSkinDark  = tft.color565(210, 170, 135);
  citizenShirtPalette[0] = tft.color565(70, 140, 210);
  citizenShirtPalette[1] = tft.color565(200, 90, 90);
  citizenShirtPalette[2] = tft.color565(240, 190, 80);
  citizenShirtPalette[3] = tft.color565(100, 180, 120);
  citizenShirtPalette[4] = tft.color565(160, 120, 200);

  float cursor = SCREEN_W + 40.0f;
  for (int i = 0; i < TOWN_COUNT; ++i) {
    towns[i].variant = i % 3;
    towns[i].width   = townWidthForVariant(towns[i].variant);
    towns[i].x       = cursor;
    initTownCitizens(towns[i], i);
    cursor += towns[i].width + randomGapForVariant(towns[i].variant);
  }
}

void townUpdate(uint32_t dt_ms) {
  float dt = dt_ms / 1000.0f;
  for (int i = 0; i < TOWN_COUNT; ++i) {
    Town &town = towns[i];
    town.x -= TOWN_SPEED_PX_PER_S * dt;
    updateTownCitizens(town, dt);
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
      initTownCitizens(town, i);
    }
  }
}

void townRender(TFT_eSprite &dst) {
  for (const Town &town : towns) {
    int x = static_cast<int>(town.x);
    if (x >= SCREEN_W || x + town.width <= 0) continue;
    drawTownVariant(dst, town.variant, x);
    drawTownCitizens(dst, town, x);
  }
}
