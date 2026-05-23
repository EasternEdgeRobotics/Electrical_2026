
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

void  readandSendFile(WiFiClient *client, String name) {
  File websiteFile = SD.open(name, FILE_READ);
  if (websiteFile) {
    byte buffer[1024];
    while (websiteFile.available()) {
      int length = websiteFile.readBytes(buffer, sizeof(buffer));
      client->write(buffer, length);
    }
    websiteFile.close();
  }
}

void writeFile(String name, char data[]){
  File dataFile = SD.open(name+".txt", FILE_WRITE);
    if (dataFile) {
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening file");
  }
}