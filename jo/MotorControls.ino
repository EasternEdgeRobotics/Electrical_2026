/*
All methods to control the motor
*/

/*
direction: true is down
speed: 0-255 pwm*/


void Move(bool direction, int speed) {

  bool TopSwitch digitalRead(topLimitSwitch);
  bool BottomSwitch digitalRead(bottomLimitSwitch);

  if ((!BottomSwitch) || (!TopSwitch))
  {
    digitalWrite(motorDown, 0);
    digitalWrite(motorUp, 0);
    break;
  }
  if (direction)
  {
    digitalWrite(motorDown, speed);
  }
  if (!direction)
  {
    digitalWrite(motorUp, speed);
  }

}

void EmergencyStop() {
  bool TopSwitch digitalRead(topLimitSwitch);
  bool BottomSwitch digitalRead(bottomLimitSwitch);
  if ((!BottomSwitch) || (!TopSwitch))
  {
    digitalWrite(motorDown, 0);
    digitalWrite(motorUp, 0);
    Serial.print("EMERGENY STOP ");
    if (!BottomSwitch)
    {
      Serial.println(" BOTTOM");
    }
    if (!TopSwitch)
    {
      Serial.println(" BOTTOM");
    }
  }
}