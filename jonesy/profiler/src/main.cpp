#include <esp_now.h>
#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <MS5837.h>
#include <Array.h>

#define SDAPIN 22
#define SCLPIN 21

//https://www.ti.com/lit/ds/symlink/drv8231a.pdf page 11 for dc
#define IN1PIN 14
#define IN2PIN 25
#define SYRINGE_MAX 27
#define SYRINGE_MIN 26
#define TIMEOUT 19000

#define SYRINGE_PULL 0
#define SYRINGE_PUSH 1
#define PWM_FREQ 1000
#define PWM_RES 8

#define POT_PIN 35

const char companyNumber[5] = {'E', 'X', '0', '6', '\0'}; //TODO: this will need to be changed

typedef struct struct_message {
  uint8_t mod = 1;
  unsigned long time;
  char text[33];
  char companyNumber[5];
} struct_message;
struct_message message;

typedef struct struct_depth {
  uint8_t mod = 2;
  unsigned long time;
  float depth;
  float pressure;
  char companyNumber[5];
} struct_depth;
struct_depth depth;

struct DepthAndPressure {
  unsigned long time;
  float depth;
  float pressure;
};

uint8_t connectivityCheck = 0;

Array<DepthAndPressure, 900> depthQueue;

uint8_t broadcastAddress[] = {0xA0, 0xA3, 0xB3, 0x2D, 0x05, 0xB4};
esp_now_peer_info_t peerInfo;

MS5837 sensor;

// AsyncWebServer server(80);

bool profiling = false;
bool priming = false;

bool recieverAvailable = false;


void conCheck(void * parameter) {
  for(;;) {
    esp_now_send(broadcastAddress, (uint8_t *) &connectivityCheck, sizeof(uint8_t));
    vTaskDelay(1000 / portTICK_RATE_MS);
  }
}

float totalDepth = 0;
float totalPressure = 0;
int profiles = 0;

int timeInRegion = 0;

void getDepthData(void * parameter) {
  for(;;) {
    if (profiling) {
      float sensor_depth = sensor.depth();
      if (sensor_depth >= 2 && sensor_depth <= 3) timeInRegion++;
      depthQueue.push_back({millis(), sensor_depth, sensor.pressure()/10});
    } else if (!profiling && (totalDepth != 0 || totalPressure != 0 || profiles != 0 || timeInRegion != 0)) {
      totalDepth = 0;
      totalPressure = 0;
      profiles = 0;
      timeInRegion = 0;
    }
    vTaskDelay(1000 * portTICK_PERIOD_MS);
  }
}

void sendDepthData(void * parameter) {
  for(;;) {
    if (recieverAvailable && !depthQueue.empty()) {
      DepthAndPressure tmp = depthQueue.front();
      depth.depth = tmp.depth;
      depth.pressure = tmp.pressure;
      depth.time = tmp.time;
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &depth, sizeof(depth));
      if (result == ESP_OK) depthQueue.remove(0);
    }
    vTaskDelay(200 / portTICK_RATE_MS);
  }
}

/**
 * @brief PID controller
 * 
 * @param target        the depth to hold in meters
 * @param margin        the plus-minus to the target that forms the range in meters
 * @param timer         how long to hold at @p target in seconds
 * @param failsafeTimer maximum time to try to hold position at target in seconds
 * @return              True if the objective was achieved, false if @p failsaveTimer was reached
 * 
 */
bool pid(float target, float margin, long timer, long failsafeTimer) {
  long startTime = millis();
  long time = startTime;
  long current = startTime;
  short dt = 2000;
  long pidTimer = 0;
  
  float previousError = 0;
  float integral = 0;
  float Kp = 0.1; // TODO fix the ratios
  float Ki = 0.1;
  float Kd = 0.1;

  float potTarget = analogRead(POT_PIN);
  float positionErrorMargin = margin*4/4095;// TODO auto calibrate the scale.  4 is max depth and 4095 is max POT Value, assumed to be at 4m deep

  // loop until too much time has pass
  while (current - startTime < failsafeTimer*1000)
  {
    current = millis();
    // only update the output every dt milliseconds
    if (current-pidTimer >= dt)
    {
      // PID based on pseudocode from Wikipedia https://en.wikipedia.org/wiki/Proportional%E2%80%93integral%E2%80%93derivative_controller#Pseudocode
      float error = target - sensor.depth();
      float proportional = error;
      integral = integral + error * 1000/dt;
      float derivative = (error - previousError) / (1000/dt);
      float output = Kp*proportional + Ki*integral + Kd*derivative;
      previousError = error;

      potTarget += output/4*4095; 
    }
    
    float potPosition = analogRead(POT_PIN);

    // too deep
    if (potPosition > potTarget+positionErrorMargin)
    {
      ledcWrite(SYRINGE_PULL, 0);
      ledcWrite(SYRINGE_PUSH, 255);
      time = current;
      continue;
    }

    // too shallow
    if (potPosition < potTarget-positionErrorMargin)
    {
      ledcWrite(SYRINGE_PULL, 255);
      ledcWrite(SYRINGE_PUSH, 0);
      time = current;
      continue;
    }

    // within range
    ledcWrite(SYRINGE_PULL, 0);
    ledcWrite(SYRINGE_PUSH, 0);
    
    if (current - time >= timer*1000)
    {
      return true;
    }
  }
  return false;
}

void profile(void * parameter) {
  profiling = true;
  message.time = millis();
  strcpy(message.text, "Profile commencing!");
  esp_now_send(broadcastAddress, (uint8_t *) &message, sizeof(message));

  // sink to the bottom
  float depth = sensor.depth();
  while (digitalRead(SYRINGE_MAX) == HIGH || depth < 3) {
    ledcWrite(SYRINGE_PULL, 255);
    ledcWrite(SYRINGE_PUSH, 0);

    vTaskDelay(100 * portTICK_RATE_MS);
    depth = sensor.depth();
  }

  while (timeInRegion < 45) {
    // Serial.println(analogRead(POT_PIN));

    float depth_val = sensor.depth();
    int pwm = max(min((int) abs((2.5 - depth_val) * 150), 255), 175);

    if (depth_val < 2.5 && digitalRead(SYRINGE_MAX) == HIGH && analogRead(POT_PIN) > 1200) {
      ledcWrite(SYRINGE_PULL, 255);
      ledcWrite(SYRINGE_PUSH, 0);
    } else if (depth_val > 2.5 && digitalRead(SYRINGE_MIN) == HIGH && analogRead(POT_PIN) < 2350) {
      ledcWrite(SYRINGE_PULL, 0);
      ledcWrite(SYRINGE_PUSH, 255);
    } else {
      ledcWrite(SYRINGE_PULL, 255);
      ledcWrite(SYRINGE_PUSH, 255);
    }

    vTaskDelay(100 * portTICK_RATE_MS);
  }
  
  // You are done. Attempt to go back up

  // To speed up going up, go to max buoyancy
  while (digitalRead(SYRINGE_MIN) == HIGH) {
    ledcWrite(SYRINGE_PULL, 0);
    ledcWrite(SYRINGE_PUSH, 255); // Full power up
    vTaskDelay(100 * portTICK_PERIOD_MS);
  }

  // You're at max buoyancy, stop the motor
  ledcWrite(SYRINGE_PULL, 0);
  ledcWrite(SYRINGE_PUSH, 0);

  vTaskDelay(10000 * portTICK_PERIOD_MS);

  profiling = false;
  message.time = millis();
  strcpy(message.text, "Profile complete!");
  esp_now_send(broadcastAddress, (uint8_t *) &message, sizeof(message));
  vTaskSuspend(xTaskGetCurrentTaskHandle());
}

void prime(void * parameter) {
  priming = true;
  message.time = millis();
  strcpy(message.text, "Priming Profiler!");
  esp_now_send(broadcastAddress, (uint8_t *) &message, sizeof(message));

  ledcWrite(SYRINGE_PULL, 0);
  ledcWrite(SYRINGE_PUSH, 255); // Full power expand
  TickType_t startTick = xTaskGetTickCount();
  // Achieve minimum buoyancy state
  while (digitalRead(SYRINGE_MIN) == 1) vTaskDelay(10 * portTICK_PERIOD_MS);
  ledcWrite(SYRINGE_PUSH, 0);

  priming = false;
  message.time = millis();
  strcpy(message.text, "Profiler ready for deployment!");
  esp_now_send(broadcastAddress, (uint8_t *) &message, sizeof(message));
  vTaskSuspend(xTaskGetCurrentTaskHandle());
}

void onSend(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS && !recieverAvailable) {
    recieverAvailable = true;
  } else if (status == ESP_NOW_SEND_FAIL && recieverAvailable) {
    recieverAvailable = false;
  }
}

void onReceive(const uint8_t * mac, const uint8_t *incomingData, int len) {
  uint8_t mod;
  memcpy(&mod, incomingData, sizeof(uint8_t));
  switch(mod) {
    case(1):
      if (profiling || priming) {
        message.time = millis();
        strcpy(message.text, "Profile already ongoing!");
        esp_now_send(broadcastAddress, (uint8_t *) &message, sizeof(message));
      }
      else xTaskCreate(profile, "Profile", 1000, NULL, 1, NULL);
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);

  Wire.begin();

  sensor.setModel(MS5837::MS5837_02BA);
  sensor.init();
  
  sensor.setFluidDensity(997);

  sensor.read();

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Unable to start ESP-NOW!");
    return;
  }

  pinMode(POT_PIN, INPUT);
  pinMode(SYRINGE_MAX, INPUT);
  pinMode(SYRINGE_MIN, INPUT);

  ledcSetup(SYRINGE_PULL, PWM_FREQ, PWM_RES);
  ledcAttachPin(IN1PIN, SYRINGE_PULL);
  
  ledcSetup(SYRINGE_PUSH, PWM_FREQ, PWM_RES);
  ledcAttachPin(IN2PIN, SYRINGE_PUSH);

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Unable to add peer!");
    return;
  }

  esp_now_register_send_cb(onSend);
  esp_now_register_recv_cb(onReceive);

  strcpy(message.companyNumber, companyNumber);
  strcpy(depth.companyNumber, companyNumber);

  // ElegantOTA.begin(&server);
  // server.begin();

  xTaskCreate(conCheck, "Connectivity Check", 1000, NULL, 1, NULL);
  xTaskCreate(getDepthData, "Get Depth Data", 1000, NULL, 1, NULL);
  xTaskCreate(sendDepthData, "Send Depth Data", 1000, NULL, 1, NULL);
  xTaskCreate(prime, "Prime Profiler", 1000, NULL, 1, NULL);
}

void loop() {
  sensor.read();
  sleep(0.05);
}