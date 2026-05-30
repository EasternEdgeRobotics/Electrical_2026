/*
direction: Profiler goes down when True
*/
void Move(bool direction) {

  int speed = 130;

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

void Stop() {
  digitalWrite(motorDown, 0);
  digitalWrite(motorUp, 0);
}
