/*
All methods to control the motor
*/

/*
direction: true is down
speed: 0-255 pwm*/
void Move(bool direction, int speed) {

  bool topSwitch = digitalRead(topLimitSwitch);
  bool bottomSwitch = digitalRead(bottomLimitSwitch);

  if (direction) {
    digitalWrite(motorUp, 0);

    if (bottomSwitch) {
      digitalWrite(motorDown, speed);
    }
    else {
      digitalWrite(motorDown, 0);
    }
  }
  else {
    digitalWrite(motorDown, 0);

    if (topSwitch) {
      digitalWrite(motorUp, speed);
    }
    else {
      digitalWrite(motorUp, 0);
    }
  }

}

void EmergencyStop() {
  bool TopSwitch = digitalRead(topLimitSwitch);
  bool BottomSwitch = digitalRead(bottomLimitSwitch);
  if (!BottomSwitch) {
    digitalWrite(motorDown, 0);

    Serial.println("EMERGENY STOP BOTTOM");
  }

  if (!TopSwitch) {
    digitalWrite(motorUp, 0);

    Serial.println("EMERGENY STOP TOP");
  }
}









