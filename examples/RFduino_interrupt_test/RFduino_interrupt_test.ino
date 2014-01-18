static volatile int woke;
int i;

int myPinCallback(uint32_t ulPin)
{
  woke++;
  return 0;  // don't exit RFduino_ULPDelay
}

void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT);
  RFduino_pinWakeCallback(2, HIGH, myPinCallback);
}

void loop() {
  int old_woke;
  if (woke) {
    old_woke = woke;
    woke = 0;
    Serial.print(i++);
    Serial.print(" ");
    Serial.print(old_woke);
    Serial.print(" ");
    Serial.println(digitalRead(2));
  }
}
