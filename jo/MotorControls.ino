const float pullM = 129.26;
const int pullB = 13352;

const float pushM = 905.57;
const int pushB = 14130;

const float travelLength = 0.087; // meters

/*
 calculates the time it takes the syringe to move a distance based on the depth
*/
long TimeToMove(float distance) {
  float depth = sensor.depth();

  if (distance > 0) {
    return (pullM*depth + pullB)/travelLength * distance;
  }
  else {
    return (pushM*depth + pushB)/travelLength * (-distance);
  }
}

/*
setup the syringe
*/
void SetupSyringe() {
  while(digitalRead(bottomLimitSwitch)) {
    Move(false, 255);
  }

  Move(true, 255);
  delay(3000);
  Move(true, 0);
  
}

const int speed = 130;
unsigned long startMovement = 0;
unsigned long movementTime = 0;
/*
direction: Profiler goes down when positive
*/
void MoveTarget(float distance) {

  bool topSwitch = digitalRead(topLimitSwitch);
  bool bottomSwitch = digitalRead(bottomLimitSwitch);

  startMovement = millis();
  movementTime = TimeToMove(distance);

  if (distance > 0) {
    digitalWrite(motorDown, 0);

    if (topSwitch) {
      digitalWrite(motorUp, speed);
    }
    else {
      digitalWrite(motorUp, 0);
    }
    
  }
  else {
    digitalWrite(motorUp, 0);

    if (bottomSwitch) {
      digitalWrite(motorDown, speed);
    }
    else {
      digitalWrite(motorDown, 0);
    }
  }
}

/*
makes sure the motor stops after the set time
*/
void TimeoutMove() {
  if (millis()-startMovement >= movementTime) {
    digitalWrite(motorUp, 0);
    digitalWrite(motorDown, 0);
  }
}

/*
direction: Profiler goes down when True
speed: 0-255 pwm*/
void Move(bool direction, int speed) {

  bool topSwitch = digitalRead(topLimitSwitch);
  bool bottomSwitch = digitalRead(bottomLimitSwitch);

  if (direction) {
    digitalWrite(motorDown, 0);

    if (topSwitch) {
      digitalWrite(motorUp, speed);
    }
    else {
      digitalWrite(motorUp, 0);
    }
    
  }
  else {
    digitalWrite(motorUp, 0);

    if (bottomSwitch) {
      digitalWrite(motorDown, speed);
    }
    else {
      digitalWrite(motorDown, 0);
    }
  }

}

void EmergencyStop() {
  bool TopSwitch = digitalRead(topLimitSwitch);
  bool BottomSwitch = digitalRead(bottomLimitSwitch);
  if (!BottomSwitch) {
    digitalWrite(motorDown, 0);

    // Serial.println("EMERGENY STOP BOTTOM");
  }

  if (!TopSwitch) {
    digitalWrite(motorUp, 0);

    // Serial.println("EMERGENY STOP TOP");
  }
}









