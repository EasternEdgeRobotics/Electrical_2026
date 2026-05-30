#include <FastLED.h>
#include <Wire.h>
#include "MS5837.h"

// pin definitions
const int topLimitSwitch = 0;
const int bottomLimitSwitch = 1;

const int motorUp = 3;
const int motorDown = 2;

const int dataPointCount = 5; // CHANGE THIS NUMBER TO TAKE MORE DATA POINTS

float pushDataArray[dataPointCount][2];
float pullDataArray[dataPointCount][2];

MS5837 sensor;

void sensorSetup() {
  Wire.begin();
  while (!sensor.init()) {
    delay(5000);
  }
  sensor.setModel(MS5837::MS5837_02BA);
  sensor.setFluidDensity(997);
  sensor.read();
}

void setup() {
  sdSetup();
  LedSetup();
  sensorSetup();
  SetLED(CRGB::DarkViolet);

  // home at the top (pull the syringe)
  while (digitalRead(topLimitSwitch)) {
    Move(true);
  }

  for (int i = 0; i < dataPointCount; i++) {
    Cycle(i);
  }

  SaveArrays();  
}

void loop() {
  LEDBlink(CRGB::Thistle);
}







