const float pullM = 129.26;
const int pullB = 13352;

const float pushM = 905.57;
const int pushB = 14130;
const int maxTime = 0.75; // in s could also be a ratio of loop time

const float travelLength = 0.087; // meters

/*
 calculates the time it takes the syringe to move a distance based on the depth
*/
long TimeToMove(float distance, float depth) {
  float moveTime=0;
  if (distance > 0) {
    moveTime=((pullM*depth + pullB)/travelLength) * distance;
  }
  else {
    moveTime=((pushM*depth + pushB)/travelLength) * (-distance);
  }

  if (moveTime>=maxTime)
  {
    moveTime=maxTime;
  } 
  return(moveTime);
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
void MoveTarget(float distance, float depth) {

  bool topSwitch = digitalRead(topLimitSwitch);
  bool bottomSwitch = digitalRead(bottomLimitSwitch);

  startMovement = millis();
  movementTime = TimeToMove(distance, depth);

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









