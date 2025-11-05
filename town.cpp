#include "town.h"

#include <algorithm>
#include <math.h>

#include "gfx.h"
#include "config.h"
#include "ground.h"
#include "tornado.h"

namespace {
struct Citizen {
  float    baseOffsetX;
  float    offsetX;
  float    bobPhase;
  float    bobSpeed;
  float    walkPhase;
  float    walkSpeed;
  float    baseWalkAmplitude;
  float    walkAmplitude;
  float    altitude;
  float    verticalVelocity;
  float    horizontalVelocity;
  bool     flying;
  bool     sheltered;
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
constexpr float TAU_F                 = 6.28318530718f;
constexpr float CITIZEN_BOB_AMPLITUDE = 2.5f;
constexpr float CITIZEN_WALK_AMPLITUDE = 6.0f;
constexpr float CITIZEN_TORNADO_TRIGGER_DIST = 38.0f;
constexpr float CITIZEN_FLY_INITIAL_VELOCITY = 32.0f;
constexpr float CITIZEN_FLY_VERTICAL_ACCEL   = 24.0f;
constexpr float CITIZEN_FLY_HORIZONTAL_PUSH  = 26.0f;
constexpr float SHELTER_FADE_RATE            = 4.2f;
constexpr float SHELTER_RECOVER_RATE         = 1.6f;
constexpr float SHELTER_LERP_SPEED           = 6.0f;

Town towns[TOWN_COUNT];

bool g_alertActive = false;

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
    cit.baseOffsetX     = std::min(desiredOffset, maxOffset);
    cit.baseOffsetX     = std::max(4.0f, cit.baseOffsetX);
    cit.offsetX         = cit.baseOffsetX;
    cit.bobPhase        = (townIndex + 1) * 0.9f + i * 1.1f;
    cit.bobSpeed        = 1.2f + 0.25f * ((town.variant + i) % 4);
    cit.walkPhase       = (townIndex * 0.5f) + i * 0.7f;
    cit.walkSpeed       = 0.8f + 0.25f * ((town.variant + i) % 3);
    float walkAmplitude = CITIZEN_WALK_AMPLITUDE - 1.0f + (i % 2) * 1.5f;
    float leftRange     = std::max(0.5f, cit.baseOffsetX - 4.0f);
    float rightRange    = std::max(0.5f, static_cast<float>(town.width - 4) - cit.baseOffsetX);
    float maxRange      = std::max(0.5f, std::min(leftRange, rightRange));
    walkAmplitude       = std::min(walkAmplitude, maxRange);
    cit.baseWalkAmplitude = walkAmplitude;
    cit.walkAmplitude     = walkAmplitude;
    cit.altitude        = 0.0f;
    cit.verticalVelocity   = 0.0f;
    cit.horizontalVelocity = 0.0f;
    cit.flying          = false;
    cit.sheltered       = false;
    cit.shirtColor      = citizenShirtPalette[(town.variant + townIndex + i) % 5];
    cit.skinColor       = ((townIndex + i) % 2 == 0) ? citizenSkinLight : citizenSkinDark;
  }
}

void updateTownCitizens(Town &town, float dt) {
  float funnelX = tornadoWorldX();
  for (int i = 0; i < town.citizenCount; ++i) {
    Citizen &cit = town.citizens[i];

    float bobSpeed = cit.flying ? cit.bobSpeed * 2.8f : cit.bobSpeed;
    if (g_alertActive) {
      bobSpeed = cit.bobSpeed;  // keep calm bob while sheltering
    }
    cit.bobPhase += bobSpeed * dt;
    if (cit.bobPhase > TAU_F) {
      cit.bobPhase = fmodf(cit.bobPhase, TAU_F);
    }

    if (g_alertActive) {
      cit.flying = false;
      cit.altitude = 0.0f;
      cit.verticalVelocity = 0.0f;
      cit.horizontalVelocity = 0.0f;

      cit.walkPhase += cit.walkSpeed * dt;
      if (cit.walkPhase > TAU_F) {
        cit.walkPhase = fmodf(cit.walkPhase, TAU_F);
      }

      float lerp = std::min(1.0f, dt * SHELTER_LERP_SPEED);
      cit.offsetX += (cit.baseOffsetX - cit.offsetX) * lerp;
      float fade = dt * SHELTER_FADE_RATE * cit.baseWalkAmplitude;
      cit.walkAmplitude = std::max(0.0f, cit.walkAmplitude - fade);

      if (!cit.sheltered && fabsf(cit.baseOffsetX - cit.offsetX) < 0.25f
          && cit.walkAmplitude <= 0.05f) {
        cit.sheltered = true;
      }
      continue;
    }

    if (cit.sheltered) {
      cit.sheltered = false;
      cit.walkAmplitude = 0.0f;
      cit.offsetX = cit.baseOffsetX;
    }

    if (!cit.flying) {
      if (cit.walkAmplitude < cit.baseWalkAmplitude) {
        float recover = dt * SHELTER_RECOVER_RATE * cit.baseWalkAmplitude;
        cit.walkAmplitude = std::min(cit.baseWalkAmplitude, cit.walkAmplitude + recover);
      }

      cit.walkPhase += cit.walkSpeed * dt;
      if (cit.walkPhase > TAU_F) {
        cit.walkPhase = fmodf(cit.walkPhase, TAU_F);
      }
      float walkOffset = sinf(cit.walkPhase) * cit.walkAmplitude;
      cit.offsetX = cit.baseOffsetX + walkOffset;

      float citizenWorldX = town.x + cit.offsetX;
      float dx = citizenWorldX - funnelX;
      if (fabsf(dx) < CITIZEN_TORNADO_TRIGGER_DIST) {
        cit.flying = true;
        cit.altitude = 2.0f;
        float proximity = CITIZEN_TORNADO_TRIGGER_DIST - fabsf(dx);
        proximity = std::max(0.0f, proximity);
        cit.verticalVelocity = CITIZEN_FLY_INITIAL_VELOCITY + proximity;
        cit.horizontalVelocity = (dx >= 0.0f ? 1.0f : -1.0f)
                               * (CITIZEN_FLY_HORIZONTAL_PUSH + proximity * 0.7f);
      }
    } else {
      cit.verticalVelocity += CITIZEN_FLY_VERTICAL_ACCEL * dt;
      cit.altitude += cit.verticalVelocity * dt;
      cit.offsetX += cit.horizontalVelocity * dt;

      // add a little extra swirl as they spin around
      float sway = sinf(cit.bobPhase * 1.5f) * 12.0f;
      cit.offsetX += sway * dt;

      if (cit.altitude > SCREEN_H + 60.0f) {
        cit.altitude = SCREEN_H + 60.0f;
      }
    }
  }
}

void drawCitizen(TFT_eSprite &dst, int x, int baseY, const Citizen &cit) {
  bool airborne = cit.flying;
  int altitudeOffset = airborne ? static_cast<int>(cit.altitude) : 0;
  float bobAmplitude = airborne ? (CITIZEN_BOB_AMPLITUDE * 0.6f)
                                : CITIZEN_BOB_AMPLITUDE;
  int bobOffset = static_cast<int>(sinf(cit.bobPhase) * bobAmplitude);
  int footY     = baseY - bobOffset - altitudeOffset;

  if (footY < -20) {
    return; // already off-screen
  }

  // ground shadow
  if (!airborne) {
    dst.drawFastHLine(x - 3, baseY + 1, 6, citizenShadow);
  } else if (cit.altitude < 18.0f) {
    int shrink = std::max(2, 6 - static_cast<int>(cit.altitude / 3.0f));
    int shadowX = x - shrink / 2;
    dst.drawFastHLine(shadowX, baseY + 1, shrink, citizenShadow);
  }

  // legs
  if (!airborne) {
    int step = static_cast<int>(sinf(cit.walkPhase * 1.2f) * 2.0f);
    dst.fillRect(x - 2, footY - 5 - step, 2, 5 + step, citizenPants);
    dst.fillRect(x,     footY - 5 + step, 2, 5 - step, citizenPants);
  } else {
    int legSwing = static_cast<int>(sinf(cit.bobPhase * 2.3f) * 3.0f);
    dst.fillRect(x - 4, footY - 7 - legSwing, 2, 5, citizenPants);
    dst.fillRect(x + 2, footY - 5 + legSwing, 2, 5, citizenPants);
  }

  // torso
  int torsoY = footY - 10 - (airborne ? 2 : 0);
  dst.fillRect(x - 3, torsoY, 6, 6, cit.shirtColor);

  // arms as small dots on the side of torso
  if (!airborne) {
    dst.drawPixel(x - 4, torsoY + 2, cit.skinColor);
    dst.drawPixel(x + 3, torsoY + 2, cit.skinColor);
  } else {
    dst.drawPixel(x - 5, torsoY + 1, cit.skinColor);
    dst.drawPixel(x + 4, torsoY + 4, cit.skinColor);
  }

  // head and hair
  int headY = torsoY - 2 - (airborne ? 1 : 0);
  dst.fillCircle(x, headY, 2, cit.skinColor);
  dst.drawFastHLine(x - 2, headY - 2, 4, citizenHair);
  dst.drawPixel(x - 2, headY - 1, citizenHair);
  dst.drawPixel(x + 1, headY - 1, citizenHair);
}

void drawTownCitizens(TFT_eSprite &dst, const Town &town, int originX) {
  int baseY = groundLine();
  for (int i = 0; i < town.citizenCount; ++i) {
    const Citizen &cit = town.citizens[i];
    if (g_alertActive && cit.sheltered) {
      continue;
    }
    if (cit.flying && cit.altitude > SCREEN_H) {
      continue;
    }
    int screenX = originX + static_cast<int>(cit.offsetX);
    if (screenX + 4 < 0 || screenX - 4 >= SCREEN_W) continue;
    drawCitizen(dst, screenX, baseY, cit);
  }
}

}  // namespace

void townInit() {
  g_alertActive = false;
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

void townSetAlert(bool active) {
  if (g_alertActive == active) return;
  g_alertActive = active;

  for (int i = 0; i < TOWN_COUNT; ++i) {
    Town &town = towns[i];
    for (int j = 0; j < town.citizenCount; ++j) {
      Citizen &cit = town.citizens[j];
      if (active) {
        cit.flying = false;
        cit.altitude = 0.0f;
        cit.verticalVelocity = 0.0f;
        cit.horizontalVelocity = 0.0f;
        cit.sheltered = false;
      } else {
        cit.sheltered = false;
        cit.walkAmplitude = 0.0f;
        cit.offsetX = cit.baseOffsetX;
      }
    }
  }
}
