/* mining-node.cpp Example sketch for recording analog signals
 *
 */
#include <Arduino.h>

#define HIT_THRESHOLD 100
const uint32_t PIN_SENSOR = A0;

const uint64_t minBeforeNextHitMs = 250;
const int64_t idealHitMs = 1000;
const uint64_t missMs = idealHitMs * 0.25;
const uint64_t stopMiningMs = 3000;

uint64_t lastHitMs = 0;
uint16_t score = 0;

void setup()
{
    Serial.begin(9600);
}

bool isMiningStarted()
{
    return lastHitMs > 0;
}

bool isMiningTimedOut(uint64_t currentTimeMs)
{
    if (lastHitMs == 0)
    {
        return false;
    }

    return currentTimeMs - lastHitMs > stopMiningMs;
}

void effectHit()
{
    Serial.println(F("Effect: Hit"));
    pinMode(PIN_SENSOR, OUTPUT);
    delay(5);
    uint32_t duration = 250;
    tone(PIN_SENSOR, 1912, duration);
    delay(duration);
    delay(5);
    analogWrite(PIN_SENSOR, 0);
    delay(5);
    pinMode(PIN_SENSOR, INPUT);
    delay(5);
}

void startMining(uint64_t currentTimeMs, uint32_t sensorValue)
{
    Serial.print(F("Start mining : "));
    Serial.println(sensorValue);
    lastHitMs = currentTimeMs;
    score = 0;

    effectHit();
}

void scoreHit(uint64_t currentTimeMs, uint32_t sensorValue)
{
    long timeBetweenHitsMs = currentTimeMs - lastHitMs;
    Serial.print(F("Score Hit at "));
    Serial.print(timeBetweenHitsMs);
    Serial.print(F(" : "));
    Serial.println(sensorValue);

    lastHitMs = currentTimeMs;
    unsigned long hitAccuracyMs = abs(idealHitMs - timeBetweenHitsMs);

    Serial.print(F("Hit accuracy: "));
    Serial.println(hitAccuracyMs);

    if (hitAccuracyMs < missMs)
    {
        score += 100 * (missMs - hitAccuracyMs) / missMs;

        Serial.print(F("Score: "));
        Serial.println(score);
    }

    effectHit();
}

bool isMiningComplete()
{
    return score > 500;
}

void effectComplete()
{
    Serial.println(F("Effect: Complete"));
}

void stopMining()
{
    Serial.println(F("Stop mining"));
    lastHitMs = 0;
    score = 0;
}

void completeMining()
{
    Serial.println(F("Mining Complete!"));
    stopMining();

    effectComplete();
}

void loop()
{
    uint64_t currentTimeMs = millis();

    uint32_t sensorValue = analogRead(PIN_SENSOR);
    if (sensorValue > HIT_THRESHOLD)
    {
        if (isMiningStarted())
        {
            scoreHit(currentTimeMs, sensorValue);
        }
        else
        {
            startMining(currentTimeMs, sensorValue);
        }
    }

    if (isMiningComplete())
    {
        completeMining();
    }

    if (isMiningTimedOut(currentTimeMs))
    {
        stopMining();
    }

    delay(1);
}
