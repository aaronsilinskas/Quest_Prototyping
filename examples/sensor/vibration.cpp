#include <Arduino.h>

const uint32_t ledPin = 13;

uint32_t vibrationCount = 0;
uint32_t lastTimeMs;

struct VibrationSensor
{
    char *name;
    uint32_t sensorPin;
    uint32_t ledPin;
    uint32_t timer;
};
const uint8_t SENSOR_COUNT = 3;
VibrationSensor sensors[SENSOR_COUNT];

void setup()
{
    Serial.begin(9600);

    sensors[0].name = "Slow";
    sensors[0].sensorPin = 12;
    sensors[0].ledPin = A1;
    sensors[0].timer = 0;
    sensors[1].name = "Medium";
    sensors[1].sensorPin = 11;
    sensors[1].ledPin = A2;
    sensors[1].timer = 0;
    sensors[2].name = "Fast";
    sensors[2].sensorPin = 10;
    sensors[2].ledPin = A3;
    sensors[2].timer = 0;

    for (uint8_t i = 0; i < SENSOR_COUNT; i++)
    {
        pinMode(sensors[i].sensorPin, INPUT_PULLUP);
        pinMode(sensors[i].ledPin, OUTPUT);
    }

    lastTimeMs = millis();
}

void loop()
{
    // update ellapsed time
    uint32_t currentTimeMs = millis();
    uint32_t ellapsedTimeMs = currentTimeMs - lastTimeMs;
    lastTimeMs = currentTimeMs;

    // update each sensor state
    for (uint8_t i = 0; i < SENSOR_COUNT; i++)
    {
        uint32_t timer = sensors[i].timer;
        if (timer > 0)
        {
            if (ellapsedTimeMs >= timer)
            {
                sensors[i].timer = 0;
                digitalWrite(sensors[i].ledPin, 0);
            }
            else
            {
                sensors[i].timer = timer - ellapsedTimeMs;
            }
        }
        else if (digitalRead(sensors[i].sensorPin) == 0)
        {
            sensors[i].timer = 1000;
            digitalWrite(sensors[i].ledPin, 1);
            Serial.print("Vibration detected on: ");
            Serial.println(sensors[i].name);
        }
    }

    delay(10);
}
