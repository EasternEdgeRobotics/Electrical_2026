

void Cycle(int index) {
  unsigned long t = millis();

  while (millis() - t <= 30*1000) {
    LEDWave(CRGB::Blue, CRGB::Olive);
  }

  SetLED(CRGB::Red2);
  delay(1*1000);
  SetLED(CRGB::Yellow2);
  delay(1*1000);
  SetLED(CRGB::Green2);

  sensor.read();

  // push the syringe
  unsigned long start = millis();
  while (digitalRead(bottomLimitSwitch)) {
    Move(false);
  }
  unsigned long end = millis();

  pushDataArray[index][0] = sensor.depth();
  pushDataArray[index][1] = end-start;

  // pull the syringe
  start = millis();
  while (digitalRead(topLimitSwitch)) {
    Move(true);
  }
  end = millis();

  pullDataArray[index][0] = sensor.depth();
  pushDataArray[index][1] = end-start;
}












