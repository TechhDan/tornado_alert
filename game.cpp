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
constexpr int SKY_STRIP_TOP       = 0;
constexpr int SKY_STRIP_H         = SCREEN_H - GROUND_HEIGHT;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 30;
constexpr int MOTOR_PWM_CHANNEL = 0;
constexpr uint32_t MOTOR_PWM_FREQ = 2000;  // Hz
constexpr uint8_t MOTOR_PWM_RES_BITS = 8;  // 0-255 duty control
constexpr uint8_t MOTOR_PWM_MAX = (1 << MOTOR_PWM_RES_BITS) - 1;
constexpr uint8_t MOTOR_RAMP_UP_STEP = 8;   // adjust to soften inrush
constexpr uint8_t MOTOR_RAMP_DOWN_STEP = 16;

TFT_eSprite skyComposite(&tft);
bool alertActive = false;
uint8_t motorDuty = 0;
}

static uint32_t lastFrame = 0;

void gameInit() {
  gfxInit();                  // make sure this calls tft.setRotation(1)

  pinMode(ALERT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);
  ledcSetup(MOTOR_PWM_CHANNEL, MOTOR_PWM_FREQ, MOTOR_PWM_RES_BITS);
  ledcAttachPin(MOTOR_PIN, MOTOR_PWM_CHANNEL);
  motorDuty = 0;
  ledcWrite(MOTOR_PWM_CHANNEL, motorDuty);

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

  static bool lastButtonReading = false;
  static uint32_t lastDebounceTime = 0;
  static bool debounceInitialized = false;

  if (!debounceInitialized) {
    lastButtonReading = buttonDown;
    debounceInitialized = true;
  }

  if (buttonDown != lastButtonReading) {
    lastDebounceTime = now;
    lastButtonReading = buttonDown;
  }

  if ((now - lastDebounceTime) > BUTTON_DEBOUNCE_MS) {
    if (buttonDown != alertActive) {
      alertActive = buttonDown;
      townSetAlert(alertActive);
    }
  }

  uint8_t targetDuty = alertActive ? MOTOR_PWM_MAX : 0;
  uint8_t rampStep = alertActive ? MOTOR_RAMP_UP_STEP : MOTOR_RAMP_DOWN_STEP;
  if (motorDuty != targetDuty) {
    if (motorDuty < targetDuty) {
      int nextDuty = static_cast<int>(motorDuty) + rampStep;
      if (nextDuty > targetDuty) nextDuty = targetDuty;
      if (nextDuty > MOTOR_PWM_MAX) nextDuty = MOTOR_PWM_MAX;
      motorDuty = static_cast<uint8_t>(nextDuty);
    } else {
      int nextDuty = static_cast<int>(motorDuty) - rampStep;
      if (nextDuty < targetDuty) nextDuty = targetDuty;
      if (nextDuty < 0) nextDuty = 0;
      motorDuty = static_cast<uint8_t>(nextDuty);
    }
    ledcWrite(MOTOR_PWM_CHANNEL, motorDuty);
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
