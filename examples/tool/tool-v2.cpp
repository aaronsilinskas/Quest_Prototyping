/* Tool V2 Prototype for Quest Game
 * Send and receive events, play sounds via piezo, motion tracking.
 *
 */

#include <Quest_IR_Transmitter.h>
#include <Quest_EventEncoder.h>
#include <Quest_IR_Receiver.h>
#include <Quest_EventDecoder.h>

// Update these values if testing on multiple devices
#define TEAM_ID 1
#define PLAYER_ID 3

// Trigger
const int PIN_TRIGGER = A5;

// IR Transmitter
Quest_IR_Transmitter irTransmitter;
Quest_EventEncoder eventEncoder = Quest_EventEncoder(irTransmitter.encodedBits, QIR_BUFFER_SIZE);
Event eventToSend;

// IR Receiver
const int PIN_IR_RECEIVER = 7;
Quest_IR_Receiver irReceiver;
Quest_EventDecoder eventDecoder = Quest_EventDecoder(irReceiver.decodedBits, QIR_BUFFER_SIZE);
Event eventReceived;

void setupTrigger()
{
    pinMode(PIN_TRIGGER, INPUT_PULLUP);
}

void setupIR()
{
    irReceiver.begin(PIN_IR_RECEIVER);

    irTransmitter.begin();

    eventToSend.teamID = TEAM_ID;
    eventToSend.playerID = PLAYER_ID;
}

void setup()
{
    Serial.begin(9600);

    Serial.println("Setting up trigger");
    setupTrigger();

    Serial.println("Setting up IR...");
    setupIR();
}

bool isTriggered()
{
    static uint64_t lastTrigger = 0;

    if (digitalRead(PIN_TRIGGER) == 0)
    {
        uint64_t currentTime = millis();
        if (currentTime - lastTrigger > 250)
        {
            lastTrigger = currentTime;
            return true;
        }
    }
    return false;
}

void sendEvent()
{
    irReceiver.disable();

    EventEncodeResult result = eventEncoder.encodeToBuffer(&eventToSend);
    if (result == EventEncoded)
    {
        irTransmitter.sendBits(eventEncoder.encodedBitCount);

        Serial.print("Sent event: ");
        Serial.print(eventToSend.eventID);
        Serial.print(" with ");
        Serial.print(eventEncoder.encodedBitCount);
        Serial.println(" encoded bits");
        printBinaryArray(irTransmitter.encodedBits, eventEncoder.encodedBitCount / 8 + 1, " ");
    }
    else
    {
        Serial.print("Error encoding event: ");
        Serial.println(result);
    }

    irReceiver.enable();
}

void sendPingWithCount(uint8_t count)
{
    eventToSend.eventID = QE_ID_PING;
    eventToSend.data[0] = count;
    eventToSend.dataLengthInBits = 8;

    sendEvent();
}

bool hasReceivedEvents()
{
    // check to see if the receiver decoded a signal
    if (irReceiver.hasSignal())
    {
        EventDecodeResult result = eventDecoder.decodeEvent(irReceiver.decodedBitCount, &eventReceived);
        irReceiver.reset();

        if (result == EventDecoded)
        {
            Serial.print("Decoded event: Team=");
            Serial.print(eventReceived.teamID);
            Serial.print(" Player=");
            Serial.print(eventReceived.playerID);
            Serial.print(" Event=");
            Serial.print(eventReceived.eventID);
            Serial.println(" Data:");
            printBinaryArray(eventReceived.data, (eventReceived.dataLengthInBits / 8) + 1, " ");

            return true;
        }
        else
        {
            Serial.print("Error decoding event: ");
            Serial.println(result);
        }
    }
    return false;
}

void loop()
{
    static uint8_t count = 0;

    if (isTriggered()) {
        sendPingWithCount(count);
        count++; // will overflow to 0
    }

    if (hasReceivedEvents())
    {
        //tone(A0, 64, 1000);
    }
}
