/* send-ir-event.ino Example sketch for Quest Event Library
 * Sends a Ping event via IR to all teams and players.
 *
 * Note: For ItsyBitsy M0, the IR LED is set to pin 4 for
 * compatibility with the Quest stack running on a single MCU.
 */

#include <Quest_IR_Transmitter.h>
#include "Quest_EventEncoder.h"

// Update these values if testing on multiple devices
#define TEAM_ID 1
#define PLAYER_ID 3

Quest_IR_Transmitter irTransmitter;
Quest_EventEncoder eventEncoder = Quest_EventEncoder(irTransmitter.encodedBits, QIR_BUFFER_SIZE);

Event event;

void setup()
{
    Serial.begin(9600);

    irTransmitter.begin();

    event.teamID = TEAM_ID;
    event.playerID = PLAYER_ID;
    event.eventID = QE_ID_PING;
}

void loop()
{
    static uint8_t count = 0;

    event.data[0] = count;
    event.dataLengthInBits = 8;

    EventEncodeResult result = eventEncoder.encodeToBuffer(&event);
    if (result == EventEncoded)
    {
        irTransmitter.sendBits(eventEncoder.encodedBitCount);

        Serial.print("Event sent: Count=");
        Serial.println(count);
        printBinaryArray(irTransmitter.encodedBits, eventEncoder.encodedBitCount / 8 + 1, " ");
    }
    else
    {
        Serial.print("Error encoding event: ");
        Serial.println(result);
    }

    count++; // will overflow to 0

    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
}
