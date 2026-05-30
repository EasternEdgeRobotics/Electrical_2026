

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
  float depth=sensor.depth();

  // push the syringe
  unsigned long start = millis();
  while (digitalRead(bottomLimitSwitch)) {
    Move(false);
  }
  Move(false);
  unsigned long end = millis();
  simpleprint("push.csv", String(depth), String(end-start)); 

  // pull the syringe
  start = millis();
  while (digitalRead(topLimitSwitch)) {
    Move(true);
  }
  Move(true);
  end = millis();
  simpleprint("pull.csv", String(depth), String(end-start)); 
}












