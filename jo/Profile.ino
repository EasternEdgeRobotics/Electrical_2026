/*
main document for Profiling methods
*/

#include <ArduinoJson.h>
#include "PID.h"

JsonDocument doc;
bool profiling = false;

int N = 0;
int currentStepIndex = 0;

String fileName = "";
unsigned long lastMeasurementTime = 0;
const int dataBufferSize = 1024;
char dataBuffer[dataBufferSize];
int dataIndex = 0;

float kP = 0.0;
float kD = 0;
float kI = 0;
float setpoint = 0;
float margin = 0;
float timer = 0;
float failsafeTimer = 0;
bool diving = false;

unsigned long startTime = 0;
unsigned long inRangeTime = 0;

const float tau = 0.02;
const float outputLimitMin = -255;
const float outputLimitMax = 255;
const float integralMin = -50;
const float integralMax = 50;
const float dt = 100;

PIDController pid;

/*
split the json with the profiling info for execution
see website/profile.json for format
*/
void SplitJson(String profileMessage) {
  // TODO: check to see if it breaks 
  DeserializationError error = deserializeJson(doc, profileMessage);

  sensor.setFluidDensity(doc["density"]);

  kP = doc["kP"];
  kD = doc["kD"];
  kI = doc["kI"];
  N = doc["profile"].size();
}

void saveData() {
  writeFile(fileName, dataBuffer);
}

void AddDataPacket() {
  unsigned long current = millis();
  if (current - lastMeasurementTime >= 1000) {
    float pressure = sensor.pressure(0.1);
    float depth = sensor.depth();
    dataIndex += snprintf(
      dataBuffer+dataIndex,
      dataBufferSize - dataIndex,
      "%s, %s, %.2f, %.2f\n",
      companyNumber, GetCurrentTime().c_str(), pressure, depth
    );

    if (dataIndex > dataBufferSize - 50) {
      saveData();
    }
  }
}

void setupDive(String input) {
  int colonIndex = input.indexOf(":");
  String numbers = input.substring(colonIndex +1);
  numbers.trim();

  int index = 0;
  float diveValues[4]; // setpoint, margin, timer, failsafeTimer

  // Split by comma
  while (numbers.length() > 0 && index < 4) {
    int commaIndex = numbers.indexOf(',');
    String token;

    if (commaIndex == -1) {
      token = numbers;  // last number
      numbers = "";
    } else {
      token = numbers.substring(0, commaIndex);
      numbers = numbers.substring(commaIndex + 1);
    }

    token.trim();
    diveValues[index] = token.toFloat();
    index++;
  }

  setpoint = diveValues[0];
  margin = diveValues[1];
  timer = diveValues[2];
  failsafeTimer = diveValues[3];

  pid = {kP, kI, kD, tau, outputLimitMin, outputLimitMax, integralMin, integralMax, (dt/1000)};
  PIDController_Init(&pid);
  startTime = millis();
  inRangeTime = startTime;
}

/*
 calls each tasks of the profile in order.  needs to also get the data points and save them each second
*/
void Profile() {
  if (profiling) {
    if (currentStepIndex == N) {
      Serial.println("profile complete");
      profiling = false;
      AddDataPacket();
      currentStepIndex = 0;
      saveData();
      return;
    }
    String input = doc["profile"][currentStepIndex];
    if(input == "sink") {
      Sink();
    }
    else if (input == "surface") {
      Surface();
    }
    else if (input.startsWith("dive")) {
      if (!diving) {
        setupDive(input);
        diving = true;
      }
      Dive();
    }
    else if (input.startsWith("led")) {
      // Remove the "led: " prefix
      int colonIndex = input.indexOf(':');
      String values = input.substring(colonIndex + 1);
      values.trim(); // remove spaces

      // Parse the three numbers
      int firstComma = values.indexOf(',');
      int secondComma = values.indexOf(',', firstComma + 1);

      int r = values.substring(0, firstComma).toInt();
      int g = values.substring(firstComma + 1, secondComma).toInt();
      int b = values.substring(secondComma + 1).toInt();

      SetLED(r, g, b);
    }
  }
}

/*
IMPORTANT:
##########
Profile() will call each method at each iteration of the main loop
do not implement infinite loops, it will stop other tasks
each time each of these method are calloed, one step should be taken,
then check if the goal has been reached.  if yes, currentStepIndex += 1
*/


/*
 sink the profiler
*/
void Sink() {
  // TODO DONE? Untested
  float sinkDepth=sensor.depth();
  Move(true, 255);
  Serial.println("sink");
  if (sinkDepth>=2)
  {
    currentStepIndex += 1;
  }
  
}

/*
 surface the profiler
*/
void Surface() {
 // TODO DONE? Untested
  float surfaceDepth=sensor.depth();
  Move(false, 255);
  Serial.println("surface");
  if (surfaceDepth<=0.02)
  {
    currentStepIndex += 1;
  }
}

/*
 perform a dive
*/
void Dive() {
  unsigned long currentTime = millis();

  if (currentTime > startTime+failsafeTimer*1000) {
    SetLED(255, 0, 0);
    currentStepIndex += 1;
    return;
  }
  else if (currentTime - inRangeTime >= timer * 1000) {
    SetLED(0, 255, 0);
    currentStepIndex += 1;
    return;
  }

  float measurement = sensor.depth();
  float output = PIDController_Update(&pid, setpoint, measurement);

  if (output >= 0) {
    Move(true, output);
  }
  else {
    Move(false, -output);
  }

  if(!((setpoint-margin < measurement) && (measurement < setpoint+margin))) {
    inRangeTime = currentTime;
  }
}

/*
 control the LEDs
*/
void SetLED(int r, int g, int b) {
  for (int i = 0; i < ledCount; i++) {
    leds[i] = CRGB(r,g,b);
  }
  FastLED.show();
  
  Serial.println("led");
  currentStepIndex += 1;
}






















