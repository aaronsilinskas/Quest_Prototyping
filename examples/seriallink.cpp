#include <Arduino.h>

const uint32_t BUTTON_PIN = 11;
uint32_t ledState = LOW;
uint32_t pingCount = 0;

void setup(void) {
  Serial.begin(9600);

  Serial1.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  if (digitalRead(BUTTON_PIN) == 0) {
    Serial.println("Button pressed, sending message");

    Serial1.print("Ping ");
    Serial1.print(pingCount);
    Serial1.println();
    Serial1.flush();

    pingCount ++;
    delay(500);
  }

  if (Serial1.available() > 0) {
    Serial.println(">>");

    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);

    while (Serial1.available() > 0) {
      Serial.println(Serial1.readStringUntil(10));
    }

    Serial.println("<<");
  }
}
