
int hour = 0;
int minute = 0;
int second = 0;
int timeDelay = 0;

void ZeroHour(int h, int m, int s) {
  hour = h;
  minute = m;
  second = s;
  timeDelay = millis();
}

String GetCurrentTime() {
  int current = millis();
  second += (current - timeDelay)/1000;
  timeDelay = current;

  while(second >= 60) {
    minute += 1;
    second -= 60;
  }

  while (minute >= 60){
    hour += 1;
    minute -= 60;
  }
  
  return String(hour) + ":" + String(minute) + ":" + String(second);
}