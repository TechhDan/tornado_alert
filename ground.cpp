#include "ground.h"
#include "gfx.h"
#include "config.h"
#include "colors.h"

static const int TILE      = 16;     // tile size
static const int GROUND_H  = 48;     // total ground height (px)
static const int SCROLL_PX_PER_SEC = 40; // speed of ground scroll
static const int SCROLL_DIR = +1;    // +1 = left→right, -1 = right→left

// One 16x16 tile sprite we’ll repeat across the strip
static TFT_eSprite groundTileTop(&tft);
static TFT_eSprite groundTileDirt(&tft);

// fractional scroll offset in pixels (0..TILE-1)
static float scrollPx = 0.0f;

static void buildGroundTile(TFT_eSprite &tile, bool includeGrass) {
  tile.setColorDepth(16);
  tile.createSprite(TILE, TILE);

  // Colors
  uint16_t grass      = tft.color565( 34,139, 34); // forest green
  uint16_t grassLite  = tft.color565( 80,200,120); // highlight
  uint16_t dirt       = tft.color565(139, 69, 19); // brown
  uint16_t dirtDark   = tft.color565(110, 50, 15); // darker specks

  // Base fill: dirt
  tile.fillSprite(dirt);

  // Speckles in dirt for texture (simple pseudo-random pattern)
  for (int y = 6; y < TILE; ++y) {
    for (int x = 0; x < TILE; ++x) {
      if (((x * 13 + y * 7) & 0x07) == 0) {
        tile.drawPixel(x, y, dirtDark);
      }
    }
  }

  if (!includeGrass) {
    return;
  }

  // Grass band on top (5 px tall)
  for (int y = 0; y < 5; ++y) {
    tile.drawFastHLine(0, y, TILE, grass);
  }
  // Little “blades” / highlights
  for (int x = 1; x < TILE-1; x += 3) {
    tile.drawPixel(x, 1, grassLite);
    tile.drawPixel(x+1, 2, grassLite);
  }
}

void groundInit() {
  buildGroundTile(groundTileTop, true);
  buildGroundTile(groundTileDirt, false);
}

void groundUpdate(uint32_t dt_ms) {
  // advance scroll phase; wrap smoothly to avoid overflow
  float delta = (SCROLL_PX_PER_SEC * (dt_ms / 1000.0f)) * SCROLL_DIR;
  scrollPx += delta;
  // keep in [0, TILE)
  if (scrollPx >= TILE) scrollPx -= TILE;
  if (scrollPx < 0)     scrollPx += TILE;
}

void groundRender() {
  const int baseY = SCREEN_H - GROUND_H;

  // Number of columns/rows to cover the ground area (+1 for wrap)
  const int cols = (SCREEN_W / TILE) + 2;
  const int rows = (GROUND_H / TILE) + 1;

  // Leftmost tile x considering scroll
  int xOffset = (int)scrollPx; // since SCROLL_DIR=+1, tiles slide right
  int startX = -xOffset;

  for (int r = 0; r < rows; ++r) {
    int y = baseY + r * TILE;
    for (int c = 0; c < cols; ++c) {
      int x = startX + c * TILE;
      if (r == 0) {
        groundTileTop.pushSprite(x, y);
      } else {
        groundTileDirt.pushSprite(x, y);
      }
    }
  }
}
