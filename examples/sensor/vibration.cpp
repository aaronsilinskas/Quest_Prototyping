#include <Arduino.h>

const int ledPin = 13;
const int PIN_SLOW_VIBRATION = 12;
const int PIN_MEDIUM_VIBRATION = 11;
const int PIN_FAST_VIBRATION = 10;

int ledState = LOW;

void setup() {
  Serial.begin(9600);

  pinMode(ledPin, OUTPUT);
  pinMode(PIN_SLOW_VIBRATION, INPUT_PULLUP);
  pinMode(PIN_MEDIUM_VIBRATION, INPUT_PULLUP);
  pinMode(PIN_FAST_VIBRATION, INPUT_PULLUP);
}

void loop() {
  // read the vibration sensors
  bool slowValue = digitalRead(PIN_SLOW_VIBRATION) == 0;
  bool mediumValue = digitalRead(PIN_MEDIUM_VIBRATION) == 0;
  bool fastValue = digitalRead(PIN_FAST_VIBRATION) == 0;

  // if any vibration triggers happened, output which ones
  if (slowValue || mediumValue || fastValue) {
      Serial.print("Vibration! ");
      Serial.print(slowValue);
      Serial.print(" ");
      Serial.print(mediumValue);
      Serial.print(" ");
      Serial.println(fastValue);

      ledState = !ledState;
      digitalWrite(ledPin, ledState);
  }

  delay(100);
}
