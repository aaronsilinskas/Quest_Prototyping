/* knock.cpp Example sketch for knock-back prop
 * Using DFPlayer MP3 player and FastLED
 *
 */

#include <Quest_IR_Transmitter.h>
#include <Quest_Trigger.h>

const int PIN_KNOCK = 9;
Quest_Trigger knockTrigger = Quest_Trigger(PIN_KNOCK, false, 50);

// Knock state
#define MAX_KNOCKS 10
#define KNOCK_WAIT_MS 3000
uint64_t knockSignal[MAX_KNOCKS];
uint64_t lastKnock = 0;
uint8_t knockIndex = 0;

void setup()
{
    Serial.begin(9600);

    pinMode(PIN_KNOCK, INPUT_PULLUP);
}

void startKnock()
{
    knockIndex = 0;
    lastKnock = millis();
}

void nextKnock()
{
    uint64_t currentTimeMs = millis();
    knockSignal[knockIndex] = currentTimeMs - lastKnock;
    knockIndex = knockIndex + 1;
    lastKnock = currentTimeMs;
}

bool isKnockingFinished()
{
    if (lastKnock == 0)
    {
        return false;
    }
    if (knockIndex >= MAX_KNOCKS)
    {
        return true;
    }

    return lastKnock + KNOCK_WAIT_MS < millis();
}

void finishKnock()
{
    Serial.println("Knock");
    for (uint8_t i = 0; i < knockIndex; i++)
    {
        Serial.println((long)knockSignal[i]);
        Serial.println("Knock");
    }
    lastKnock = 0;
    knockIndex = 0;
}

void loop()
{
    if (knockTrigger.isTriggered())
    {
        if (lastKnock == 0)
        {
            Serial.println("Start knock...");

            startKnock();
        }
        else
        {
            Serial.println("Next knock...");

            nextKnock();
        }
        knockTrigger.delayNextTrigger(200);
    }

    if (isKnockingFinished())
    {
        Serial.println("Knocking done.");

        finishKnock();
    }
}
