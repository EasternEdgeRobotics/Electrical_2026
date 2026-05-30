void CyclePull() {
  // push the syringe
  sensor.read();
  float startDepth=sensor.depth();

  unsigned long start = millis();
  while (digitalRead(bottomLimitSwitch)) {
    Move(false);
  }
  Move(false);

  unsigned long end = millis();
  sensor.read();
  float endDepth = sensor.depth();
  
  simpleprint("push.csv", String((startDepth+endDepth)/2), String(end-start)); 
}

void CyclePush() {
  // pull the syringe
  sensor.read();
  float startDepth=sensor.depth();

  unsigned long start = millis();
  while (digitalRead(topLimitSwitch)) {
    Move(true);
  }
  Move(true);

  unsigned long end = millis();
  sensor.read();
  float endDepth = sensor.depth();

  simpleprint("pull.csv", String((startDepth+endDepth)/2), String(end-start)); 
}


void Cycle(int index) {
  unsigned long t = millis();

  while (millis() - t <= 10*1000) {
    LEDWave(CRGB::Blue, CRGB::Olive);
  }

  SetLED(CRGB::Red2);
  delay(1*1000);
  SetLED(CRGB::Yellow2);
  delay(1*1000);
  SetLED(CRGB::Green2);

  for(int i = 0; i < 5; i++) {
    CyclePull();
    CyclePush();
  }

  
}












