
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

void writeFile(String name, char data[]){
  File dataFile = SD.open(name, FILE_WRITE);
    if (dataFile) {
      dataFile.println(data);
      dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening file");
  }
}

void SaveArrays() {
  const int size = 1024;
  char data[size];
  int index = 0;

  for (int i = 0; i < dataPointCount; i++) {
    index += sprintf(&data[index], "%f,%f\n", 
                     pushDataArray[i][0], 
                     pushDataArray[i][1]);
  }
  writeFile("push_data.csv", data);

  memset(data, 0, size);
  index = 0;

  for (int i = 0; i < dataPointCount; i++) {
    index += sprintf(&data[index], "%f,%f\n", 
                     pullDataArray[i][0], 
                     pullDataArray[i][1]);
  }

  writeFile("pull_data.csv", data);
}