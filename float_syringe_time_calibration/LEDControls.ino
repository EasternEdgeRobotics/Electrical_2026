const int ledPin = 5;
const int ledCount = 13;
CRGB leds[ledCount];

uint8_t pos = 0;
uint8_t pos2 = 7;
uint8_t fadeAmount = 100;   // higher = faster fade

unsigned long lastLEDMove = 0;
uint16_t LEDMoveInterval = 100;   // ms between steps

unsigned long lastLEDBlink = 0;
uint16_t LEDBlinkInterval = 1000;
bool blinkStatus = false;

void LedSetup() {
  FastLED.addLeds<WS2812B, ledPin, GRB>(leds, ledCount);
  FastLED.setBrightness(100);
}

void SetLED(CRGB colour) {
  for (int i = 0; i < ledCount; i++) {
    leds[i] = colour;
  }
  FastLED.show();
}

void LEDWave(CRGB colour1, CRGB colour2) {
  if (millis() - lastLEDMove >= LEDMoveInterval) {
    lastLEDMove = millis();

    fadeToBlackBy(leds, ledCount, fadeAmount);

    if (pos == ledCount) {
      pos = 0;
    }

    if (pos2 == ledCount) {
      pos2 = 0;
    }

    leds[pos] = colour1;
    leds[pos2] = colour2;
    pos++;
    pos2++;

    FastLED.show();
  }
}

void LEDBlink(CRGB colour) {
  if (millis() - lastLEDBlink >= LEDBlinkInterval) {
    lastLEDBlink = millis();

    if(blinkStatus) {
      SetLED(CRGB::Gray0);
    }
    else {
      SetLED(colour);
    }

    FastLED.show();
  }
}






