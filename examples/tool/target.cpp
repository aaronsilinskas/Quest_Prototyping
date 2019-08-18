/* Tool V2 Prototype for Quest Game
 * Send and receive events, play sounds via piezo, motion tracking.
 *
 */

#include <Quest_IR_Transmitter.h>
#include <Quest_EventEncoder.h>
#include <Quest_IR_Receiver.h>
#include <Quest_EventDecoder.h>
#include <Adafruit_NeoPixel.h>

// Update these values if testing on multiple devices
#define TEAM_ID 2
#define PLAYER_ID 1

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

// NeoPixels
#define PIXEL_PIN 10
#define PIXEL_COUNT 12
Adafruit_NeoPixel pixels(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

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

void setupNeoPixels()
{
    pixels.begin();
    pixels.setBrightness(16);
    pixels.show();
}

void setup()
{
    Serial.begin(9600);

    Serial.println("Setting up trigger");
    setupTrigger();

    Serial.println("Setting up IR...");
    setupIR();

    Serial.println("Setting up NeoPixels...");
    setupNeoPixels();
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

void setPixelColor(int pixel, uint8_t maxNoise)
{
    Serial.print("Noise: ");
    Serial.println(maxNoise);

    uint8_t minNoise = 60;
    uint16_t clippedNoise = 0;
    if (maxNoise > minNoise)
    {
        clippedNoise = 255 * (maxNoise - minNoise) / (255 - minNoise);
    }
    pixels.setPixelColor(pixel, pixels.Color(clippedNoise, 255 - clippedNoise, 0));
}

void loop()
{
    static uint8_t count = 0;
    static uint8_t nextPixel = 0;

    if (isTriggered())
    {
        sendPingWithCount(count);
        count++; // will overflow to 0
    }

    if (hasReceivedEvents())
    {
        if (nextPixel == 0)
        {
            pixels.clear();
        }
        setPixelColor(nextPixel, irReceiver.decodeMaxNoise);
        pixels.show();

        nextPixel += 1;
        if (nextPixel >= PIXEL_COUNT)
        {
            nextPixel = 0;
        }
    }
}
