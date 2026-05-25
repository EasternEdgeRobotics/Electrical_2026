/*
 Main.  execute top level calls
*/

#include <WiFiNINA.h>
#include <Wire.h>
#include "MS5837.h"
#include <FastLED.h>

// pin definitions
const int topLimitSwitch = 0;
const int bottomLimitSwitch = 1;

const int ledPin = 5;
const int ledCount = 13;
CRGB leds[ledCount];

const int motorUp = 3;
const int motorDown = 2;

MS5837 sensor;

const char companyNumber[] = "EX00";

/*
seperate all initialization functions in their respective tabs
*/
void setup() {

  Serial.begin(9600);
  Serial.println("Starting up");
  

  pinMode(topLimitSwitch, INPUT);
  pinMode(bottomLimitSwitch, INPUT);

  FastLED.addLeds<WS2812, ledPin, GRB>(leds, ledCount);
  SetLED(0, 10, 5); //Light to show its on
  pinMode(motorUp, OUTPUT);
  pinMode(motorDown, OUTPUT);

  // Initialize pressure sensor
  // Returns true if initialization was successful
  // We can't continue with the rest of the program unless we can initialize the sensor
  Wire.begin();
  while (!sensor.init()) {
    delay(5000);
  }
  sensor.setModel(MS5837::MS5837_02BA);
  sensor.setFluidDensity(997);
  
  /*I belive this should be set upon getting the profilier for data, defaults to seawater */
  //sensor.setFluidDensity(997); // kg/m^3 (freshwater, 1029 for seawater)

  sdSetup();
  WifiSetup();

  SetupSyringe();
}

/*
separate the loop code into functions in their respective tabs.  the functions themselves should not loop since loop() is an infinite loop
*/
void loop() {
  EmergencyStop();
  WifiLoop();
  Profile();
}
