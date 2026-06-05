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
float depthError = 0;
float setpoint = 0;
float margin = 0;
float timer = 0;
float failsafeTimer = 0;
bool diving = false;

unsigned long startTime = 0;
unsigned long inRangeTime = 0;
unsigned long pidLastUpdateTime = 0;
float pidOutput = 0;

const float tau = 0.02;

//CLAMPING
const float outputLimitMin = -255;
const float outputLimitMax = 255;

const float integralMin = -50;
const float integralMax = 50;
const float dt = 750;

PIDController pid;

/*
split the json with the profiling info for execution
see website/profile.json for format
*/
void SplitJson(String profileMessage) {
  // TODO: check to see if it breaks 
  DeserializationError error = deserializeJson(doc, profileMessage);

  Serial.print("density: ");
  Serial.println(doc["density"].as<float>());
  // sensor.setFluidDensity(doc["density"]);

  kP = doc["kP"];
  kD = doc["kD"];
  kI = doc["kI"];
  N = doc["profile"].size();
}

void saveData() {
  writeFile(fileName, dataBuffer);
  memset(dataBuffer, 0, dataBufferSize);
  dataIndex = 0;
}

void AddDataPacket() {
  unsigned long current = millis();
  if (current - lastMeasurementTime >= 100) {
    lastMeasurementTime = current;
    float depth = sensor.depth();
    dataIndex += snprintf(
      dataBuffer+dataIndex,
      dataBufferSize - dataIndex,
      "%s, %s, %.2f, %.2f\n",
      companyNumber, GetCurrentTime().c_str(), pidOutput, depth
    );
    
    // Serial.print("depth: ");
    // Serial.println(depth);
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

void SetNextProfileStep() {

  if (!profiling) {
    return;
  }
  saveData();

  currentStepIndex += 1;

  char data[128];
  if (currentStepIndex == N) {
    profiling = false;
    currentStepIndex = 0;
    saveData();
    Serial.println("profile complete");

    snprintf(data, sizeof(data), "# profile complete");
    writeFile(fileName, data);
    return;
  }

  String step = doc["profile"][currentStepIndex];

  snprintf(data, sizeof(data), "\n# %s", step.c_str());
  writeFile(fileName, data);
}

/*
 calls each tasks of the profile in order.  needs to also get the data points and save them each second
*/
void Profile() {
  if (profiling) {
    AddDataPacket();
    sensor.read();
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
      SetNextProfileStep();
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
  pidOutput = 0;
  float sinkDepth=sensor.depth();
  Move(true, 255);
  if (sinkDepth>=2)
  {
    SetNextProfileStep();
  }

  LEDBlink(CRGB::DarkSlateBlue);
}

/*
 surface the profiler
*/
void Surface() {
  pidOutput = 0;
  float surfaceDepth=sensor.depth();
  Move(false, 255);
  if (surfaceDepth<=0.02)
  {
    SetNextProfileStep();
  }

  LEDBlink(CRGB::DarkGreen);
}

/*
 perform a dive
*/
void Dive() {
  unsigned long currentTime = millis();

  // failled, timeout
  if (currentTime > startTime+failsafeTimer*1000) {
    SetLED(255, 0, 0);
    diving = false;
    SetNextProfileStep();
    return;
  }
  // success
  else if (currentTime - inRangeTime >= timer * 1000) {
    SetLED(0, 255, 0);
    diving = false;
    SetNextProfileStep();
    return;
  }
  float measurement = sensor.depth();
  if(millis()-pidLastUpdateTime >= (dt-velocityInterval))
  {
    // Serial.println("velocityInterval Dive1");
    if (!velocityTimeMeasured)
    {
      // Serial.println("velocityInterval Dive2");
      depth1=measurement;
      depth1Time=millis();
      velocityTimeMeasured=true;
    }
  }
  
  if(millis()-pidLastUpdateTime >= dt) {

    depthError=DepthError(setpoint, measurement); //calcualates depth error from setpoint

    float targetVelocity=VelocityMap(depthError); //convertes depth error to velocity error with map

    float currentVelocity=CurrentVelocity(measurement); //measures current velocity

    pidLastUpdateTime = millis();
    Serial.print("targetVelocity:");
    Serial.println(targetVelocity);

    pidOutput = PIDController_Update(&pid, targetVelocity, currentVelocity); //returns unfiltered PID output

    MoveTarget(pidOutput,measurement); //huge chain here
    Serial.print("pidOutput:");
    Serial.println(pidOutput,5);
  }
  
  TimeoutMove();
  if(!((setpoint-margin < measurement) && (measurement < setpoint+margin))) {
    inRangeTime = currentTime;
  }
  // Serial.print("pidOutput: ");
  // Serial.println(pidOutput);
  if(pidOutput > 0) {
    // Serial.println("pidOutput>0");
    LEDWave(1, CRGB::Purple, CRGB::DeepPink2);
  }
  else if(pidOutput < 0) {
    // Serial.println("pidOutput<0");
    LEDWave(-1, CRGB::Purple, CRGB::DeepPink2);
  }
}






















