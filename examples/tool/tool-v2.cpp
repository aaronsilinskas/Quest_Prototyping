/* Tool V2 Prototype for Quest Game
 * Send and receive events, play sounds via piezo, motion tracking.
 *
 */

#include <Wire.h>
#include <SPI.h>
#include <Quest_IR_Transmitter.h>
#include <Quest_EventEncoder.h>
#include <Quest_IR_Receiver.h>
#include <Quest_EventDecoder.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_NeoPixel.h>

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

// Accelerometer
Adafruit_LIS3DH lisIMU = Adafruit_LIS3DH();
// Adjust this number for the sensitivity of the 'click' force
// this strongly depend on the range! for 16G, try 5-10
// for 8G, try 10-20. for 4G try 20-40. for 2G try 40-80
#define CLICKTHRESHHOLD 80

// NeoPixels
#define PIXEL_PIN 12
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

void setupIMU()
{
    if (!lisIMU.begin(0x18))
    {
        Serial.println("Could not start LIS3DH!");
        return;
    }
    lisIMU.setRange(LIS3DH_RANGE_2_G);
    // 0 = turn off click detection & interrupt
    // 1 = single click only interrupt output
    // 2 = double click only interrupt output, detect single click
    // Adjust threshhold, higher numbers are less sensitive
    lisIMU.setClick(2, CLICKTHRESHHOLD);
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

    Serial.println("Setting up accelerometer...");
    setupIMU();

    Serial.println("Setting up NeoPixels...");
    setupNeoPixels();
}

bool isTriggered()
{
    static uint64_t lastTrigger = 0;

    uint64_t currentTime = millis();
    if (currentTime - lastTrigger < 250)
    {
        return false;
    }

    if (digitalRead(PIN_TRIGGER) == 0)
    {
        lastTrigger = millis();
        return true;
    }

    uint8_t click = lisIMU.getClick();
    if (click & 0x20)
    {
        lastTrigger = millis();
        return true;
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

void toneCheck()
{
    tone(A0, 64, 50);
    delay(50);
    tone(A0, 92, 50);
    delay(50);
    tone(A0, 128, 50);
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

void updateNeoPixels() {
    static uint8_t pixel = 0;
    static uint64_t lastChange = 0;
    if (millis() - lastChange > 500) {
        lastChange = millis();

        if (pixels.getPixelColor(pixel) > 0) {
            pixels.setPixelColor(pixel, 0);
        } else {
            pixels.setPixelColor(pixel, pixels.Color(128, 0, 128));
        }

        pixels.show();
        pixel ++;
        if (pixel >= PIXEL_COUNT) {
            pixel = 0;
        }
    }
}

void loop()
{
    static uint8_t count = 0;

    if (isTriggered())
    {
        sendPingWithCount(count);
        count++; // will overflow to 0
    }

    if (hasReceivedEvents())
    {
        toneCheck();
    }

    //updateNeoPixels();
}
