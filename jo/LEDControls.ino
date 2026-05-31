
uint8_t pos = 0;
uint8_t pos2 = 7;
uint8_t fadeAmount = 100;   // higher = faster fade

unsigned long lastLEDMove = 0;
uint16_t LEDMoveInterval = 100;   // ms between steps

unsigned long lastLEDBlink = 0;
uint16_t LEDBlinkInterval = 1000;
bool blinkStatus = false;

void SetLED(CRGB colour) {
  for (int i = 0; i < ledCount; i++) {
    leds[i] = colour;
  }
  FastLED.show();
}

void SetLED(int r, int g, int b) {
  for (int i = 0; i < ledCount; i++) {
    leds[i] = CRGB(r,g,b);
  }
  FastLED.show();

}

void LEDWave(int direction, CRGB colour1, CRGB colour2) {
  if (millis() - lastLEDMove >= LEDMoveInterval) {
    lastLEDMove = millis();

    fadeToBlackBy(leds, ledCount, fadeAmount);

    leds[pos] = colour1;
    leds[pos2] = colour2;

    pos  = (pos  + direction + ledCount) % ledCount;
    pos2 = (pos2 + direction + ledCount) % ledCount;

    FastLED.show();
  }
}

void LEDBlink(CRGB colour) {
  if (millis() - lastLEDBlink >= LEDBlinkInterval) {
    lastLEDBlink = millis();

    if(blinkStatus) {
      SetLED(CRGB::Gray0);
      blinkStatus = false;
    }
    else {
      SetLED(colour);
      blinkStatus = true;
    }

    FastLED.show();
  }
}