/* prop.cpp Example sketch for remote Prop FX
 * Using DFPlayer MP3 player and FastLED
 *
 */

#include <Quest_IR_Transmitter.h>
#include "Quest_EventEncoder.h"

// NOTE: CHANGE THIS FOR EACH PROP OR SHARE ID FOR MULTI-TRIGGER
const int PIN_HARD_BUTTON = 9;
const int PIN_SOFT_BUTTON = 10;
const int PIN_ALLIGATOR_CLIP = 11;
const int MAX_PIN = 11;

bool lastButtonState[MAX_PIN + 1];

// IR
Quest_IR_Transmitter irTransmitter;
Quest_EventEncoder eventEncoder = Quest_EventEncoder(irTransmitter.encodedBits, QIR_BUFFER_SIZE);
Event event;

void setup()
{
    Serial.begin(9600);

    // set up triggers
    pinMode(PIN_HARD_BUTTON, INPUT_PULLUP);
    pinMode(PIN_SOFT_BUTTON, INPUT_PULLUP);
    pinMode(PIN_ALLIGATOR_CLIP, INPUT_PULLUP);

    // set up IR
    irTransmitter.begin();

    event.teamID = 0;
    event.eventID = QE_ID_PING;
    event.dataLengthInBits = 0;
}

bool isTriggered(int pin)
{
    static long lastPress = millis();

    if (millis() - lastPress < 500)
    {
        return false;
    }

    int currentState = digitalRead(pin);
    if (currentState == HIGH)
    {
        if (!lastButtonState[pin]) {
            Serial.println("Released");
        }
        lastButtonState[pin] = true;
        return false;
    }

    if (!lastButtonState[pin])
    {
        return false;
    }

    lastPress = millis();
    lastButtonState[pin] = false;
    return true;
}

void sendEvent(byte playerId)
{
    event.playerID = playerId;
    EventEncodeResult result = eventEncoder.encodeToBuffer(&event);
    if (result == EventEncoded)
    {
        irTransmitter.sendBits(eventEncoder.encodedBitCount);

        Serial.print("Event sent to player ID:");
        Serial.println(playerId);
        printBinaryArray(irTransmitter.encodedBits, eventEncoder.encodedBitCount / 8 + 1, " ");
    }
}

void loop()
{
    if (isTriggered(PIN_HARD_BUTTON))
    {
        sendEvent(1);
    }
    else if (isTriggered(PIN_SOFT_BUTTON))
    {
        sendEvent(2);
    }
    else if (isTriggered(PIN_ALLIGATOR_CLIP))
    {
        sendEvent(3);
    }
}
