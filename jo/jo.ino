/*
 Main.  execute top level calls
*/

#include <Wire.h>
#include "MS5837.h"

// pin definitions
const int topLimitSwitch = 0;
const int bottomLimitSwitch = 1;
const int led = 2;
const int motor = 3

MS5837 sensor;


/*
seperate all initialization functions in their respective tabs
*/
void setup() {
  
  WifiSetup();

}

/*
separate the loop code into functions in their respective tabs.  the functions themselves should not loop since loop() is an infinite loop
*/
void loop() {

  WifiLoop();
}
