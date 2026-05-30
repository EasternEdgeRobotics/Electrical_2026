
#include <SD.h>

const int chipSelect = 4;

void sdSetup() {
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  
  Serial.println("card initialized.");
}

void simpleprint(String name, String depth, String startend) {
  File datafile = SD.open(name, FILE_WRITE);
  if (datafile) {
    datafile.print(depth);
    datafile.print(",");
    datafile.println(startend);
    datafile.close();
    Serial.println("Write complete");
  }
  else {
  Serial.println("Write fail");
  }

}










