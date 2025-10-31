#include "game.h"

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("BOOT: hello from setup()");
  gameInit();
}

void loop() {
  // static uint32_t last = 0;
  // if (millis() - last > 1000) {     // print once per second
  //   last = millis();
  //   Serial.println("loop: alive");
  // }
  gameUpdate();
}
