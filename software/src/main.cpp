#include <Arduino.h>

void setup() {
  pinMode(0, INPUT_PULLUP);

  pinMode(15, OUTPUT);
  digitalWrite(15, LOW);

  Serial.begin(9600);
}

void loop() {
  Serial.println(digitalRead(0));
  delay(500);
}