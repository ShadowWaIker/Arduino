int chargePin = 8;
int lightPin = 9;

void setup() {
  pinMode(chargePin, OUTPUT);
  pinMode(lightPin, OUTPUT);
  digitalWrite(chargePin, HIGH);
}

void loop() {
  digitalWrite(lightPin, LOW);
  delay(1000);
  digitalWrite(lightPin, HIGH);
}
