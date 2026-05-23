
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
  unsigned long totalSeconds = millis() / 1000;

  int h = totalSeconds / 3600;
  int m = (totalSeconds % 3600) / 60;
  int s = totalSeconds % 60;

  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", h, m, s);

  return String(buffer);
}
