// game.cpp
#include <Arduino.h>

#include "game.h"
#include "gfx.h"
#include "ground.h"
#include "mountains.h"
#include "clouds.h"
#include "tornado.h"
#include "town.h"
#include "config.h"
#include "colors.h"

namespace {
constexpr int SKY_STRIP_TOP = 0;
constexpr int SKY_STRIP_H   = SCREEN_H - GROUND_HEIGHT;

TFT_eSprite skyComposite(&tft);
bool alertActive = false;
}

static uint32_t lastFrame = 0;

void gameInit() {
  gfxInit();                  // make sure this calls tft.setRotation(1)

  pinMode(ALERT_BUTTON_PIN, INPUT_PULLUP);

  // Paint a valid first frame (sky + ground) so there are no leftovers
  tft.fillScreen(SKY_BLUE(tft));
  groundInit();
  groundRender();

  // Create the shared composite sprite for the sky band.
  skyComposite.setColorDepth(8);
  if (!skyComposite.createSprite(SCREEN_W, SKY_STRIP_H)) {
    Serial.println("[game] skyComposite createSprite FAILED");
  } else {
    Serial.println("[game] skyComposite ready");
  }

  // Init clouds (mountains/tornado can be enabled later)
  cloudsInit();
  mountainsInit();
  townInit();
  tornadoInit();

  // Prime the composite so the first frame is flicker-free.
  skyComposite.fillSprite(SKY_BLUE(tft));
  cloudsRender(skyComposite);
  mountainsRender(skyComposite);
  tornadoRender(skyComposite);
  skyComposite.pushSprite(0, SKY_STRIP_TOP);

  lastFrame = millis();
}

void gameUpdate() {
  uint32_t now = millis();
  uint32_t dt  = now - lastFrame;
  if (dt < FRAME_MS) return;
  lastFrame = now;

  bool buttonDown = (digitalRead(ALERT_BUTTON_PIN) == LOW);
  if (buttonDown != alertActive) {
    alertActive = buttonDown;
    townSetAlert(alertActive);
  }

  // Update
  cloudsUpdate(dt);
  mountainsUpdate(dt);
  townUpdate(dt);
  tornadoUpdate(dt);
  groundUpdate(dt);

  // Draw one frame by rebuilding the shared composite then pushing it once.
  skyComposite.fillSprite(SKY_BLUE(tft));
  cloudsRender(skyComposite);
  mountainsRender(skyComposite);
  townRender(skyComposite);
  tornadoRender(skyComposite);

  tft.startWrite();
  skyComposite.pushSprite(0, SKY_STRIP_TOP);
  groundRender();     // opaque, drawn last

  tft.endWrite();
}
