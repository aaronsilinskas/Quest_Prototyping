/* prop.cpp Example sketch for remote Prop FX
 * Using DFPlayer MP3 player and FastLED
 *
 */

#include <Quest_IR_Transmitter.h>
#include <Quest_Trigger.h>
#include "Quest_EventEncoder.h"

const int PIN_HARD_BUTTON = 9;
Quest_Trigger hardButton = Quest_Trigger(PIN_HARD_BUTTON, false, 50);
const int PIN_SOFT_BUTTON = 10;
Quest_Trigger softButton = Quest_Trigger(PIN_SOFT_BUTTON, false, 50);
const int PIN_ALLIGATOR_CLIP = 11;
Quest_Trigger alligatorButton = Quest_Trigger(PIN_ALLIGATOR_CLIP, false, 50);
const int MAX_PIN = 11;

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
    if (hardButton.isTriggered())
    {
        sendEvent(1);
        delay(500);
    }
    else if (softButton.isTriggered())
    {
        sendEvent(2);
        delay(500);
    }
    else if (alligatorButton.isTriggered())
    {
        sendEvent(3);
        delay(500);
    }
}
