/*
 Main.  execute top level calls
*/

#include <Wire.h>
#include "MS5837.h"

// pin definitions
const int topLimitSwitch = 0;
const int bottomLimitSwitch = 1;
const int led = 2;
const int motorPWM = 3;
const int motorDirection = 4;  // CURRENTLY ASSUMING HIGH IS DOWN AND LOW IS UP

MS5837 sensor;


/*
seperate all initialization functions in their respective tabs
*/
void setup() {

  pinMode(topLimitSwitch, INPUT);
  pinMode(bottomLimitSwitch, INPUT);
  pinMode(led, OUTPUT);
  pinMode(motorPWM, OUTPUT);
  pinMode(motorDirection, OUTPUT);
  
  WifiSetup();

  // Initialize pressure sensor
  // Returns true if initialization was successful
  // We can't continue with the rest of the program unless we can initialize the sensor
  Wire.begin();
  while (!sensor.init()) {
    delay(5000);
  }

  sensor.setFluidDensity(997); // kg/m^3 (freshwater, 1029 for seawater)
}

/*
separate the loop code into functions in their respective tabs.  the functions themselves should not loop since loop() is an infinite loop
*/
void loop() {

  WifiLoop();
}
