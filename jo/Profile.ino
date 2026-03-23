/*
main document for Profiling methods
*/

#include <ArduinoJson.h>

JsonDocument doc;
bool profiling = false;

int N = 0;
int currentStepIndex = 0;

float kP = 0.0;
float kD = 0;
float kI = 0;
float diveValues[4]; // setpoint, margin, timer, failsafeTimer
bool diving = false;

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

void setupDive(String input) {
  int colonIndex = input.indexOf(":");
  String numbers = input.substring(colonIndex +1);
  numbers.trim();

  int index = 0;

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
}

/*
 calls each tasks of the profile in order.  needs to also get the data points and save them each second
*/
void Profile() {
  if (profiling) {
    if (currentStepIndex == N) {
      profiling = false;
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
      SetLED(input);
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
  // TODO
  Serial.println("sink");
  currentStepIndex += 1;
}

/*
 surface the profiler
*/
void Surface() {
 // TODO
 Serial.println("surface");
  currentStepIndex += 1;
}

/*
 perform a dive
*/
void Dive() {
  // TODO
  Serial.println("dive");
  currentStepIndex += 1;
}

/*
 control the LEDs
*/
void SetLED(String input) {
  // TODO
  Serial.println("led");
  currentStepIndex += 1;
}






















