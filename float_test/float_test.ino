#include <FastLED.h>

const int motor1 = 2;
const int motor2 = 3;

const int uplimit = 0;
const int downlimit = 1;

const int ledPin = 5;
const int ledCount = 13;
CRGB leds[ledCount];

void setup() {
  FastLED.addLeds<WS2812, ledPin, GRB>(leds, ledCount);

  for (int i = 0; i < ledCount; i++) {
    leds[i] = CRGB ( 0, 0, 255);
    FastLED.show();
    delay(100);
  }

  pinMode(motor1, OUTPUT);
  pinMode(motor2, OUTPUT);

  pinMode(uplimit, INPUT);
  pinMode(downlimit, INPUT);

  Serial.begin(9600);

  Serial.println("testing top limit");
  if(digitalRead(uplimit) == LOW) {
    Serial.println("ERROR.  TOP LIMIT DEFECTIVE");
    leds[0] = CRGB(255,0,0);
    FastLED.show();
    while (true) {}
  }
  Serial.println("press top limit");
  while(digitalRead(uplimit) == HIGH) {
    if (digitalRead(uplimit) == LOW) {
    break;
    }
  }
  leds[0] = CRGB(0,255,0);
  FastLED.show();

  Serial.println(("testing bottom limit"));
  if(digitalRead(downlimit) == LOW) {
    Serial.println("ERROR.  BOTTOM LIMIT DEFECTIVE");
    leds[1] = CRGB(255,0,0);
    FastLED.show();
    while (true) {}
  }
  Serial.println("press bottom limit");
  while(digitalRead(downlimit) == HIGH) {
    if (digitalRead(downlimit) == LOW) {
    break;
    }
  }

  leds[1] = CRGB(0,255,0);
  FastLED.show();

  Serial.println("limit test completed");
  Serial.println("Starting motor test in ");

  for(int i = 10; i != 5; i--) {
    Serial.println(i-5);
    delay(1000);
    leds[i] = CRGB(200,20,255);
    FastLED.show();
  }

  Serial.println("direction 1");
  leds[2] = CRGB(0,255,0);
  FastLED.show();
  while ((digitalRead(downlimit) == HIGH) && (digitalRead(uplimit) == HIGH)) {
    analogWrite(motor1, 255);
    analogWrite(motor2, 0);
  }
  analogWrite(motor1, 0);
  
  leds[2] = CRGB(0,0,255);
  FastLED.show();

  delay(3000);

  Serial.println("direction 2");
  leds[3] = CRGB(0,255,0);
  FastLED.show();

  //move until the limit switch is no longer pressed
  while ((digitalRead(downlimit) == LOW) || (digitalRead(uplimit) == LOW)) {
    analogWrite(motor1, 0);
    analogWrite(motor2, 255);
  }
  delay(2000);

  while (digitalRead(downlimit) == HIGH && digitalRead(uplimit) == HIGH) {
    analogWrite(motor1, 0);
    analogWrite(motor2, 255);
  }
  analogWrite(motor2, 0);
  leds[3] = CRGB(0,0,255);
  FastLED.show();

  Serial.println("centering relic");
  analogWrite(motor1, 255);
  delay(2000);
  analogWrite(motor1, 0);

  Serial.println("Test Completed");
}

void loop() {
  // put your main code here, to run repeatedly:

}
