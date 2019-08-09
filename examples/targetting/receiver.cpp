/* receive-ir-event.ino Example sketch for Quest Event Library
 * Receives a Ping event via IR and blinks the LED.
 *
 */

#include <Quest_IR_Receiver.h>
#include "Quest_EventDecoder.h"
#include <Adafruit_NeoPixel.h>

#define TEAM_ID 1
#define PLAYER_ID 3

const int PIN_IR_RECEIVER = 7;
Quest_IR_Receiver irReceiver;
Quest_EventDecoder eventDecoder = Quest_EventDecoder(irReceiver.decodedBits, QIR_BUFFER_SIZE);

Event event;

#define PIXEL_PIN    10
#define PIXEL_COUNT  12
Adafruit_NeoPixel pixels(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
    Serial.begin(9600);

    // start to listen for IR signals on the given pin
    irReceiver.begin(PIN_IR_RECEIVER);
    irReceiver.enableBlink(true);

    pixels.begin();
    pixels.show();
}

void loop()
{
    // check to see if the receiver decoded a signal
    if (irReceiver.hasSignal())
    {
        // data was received, dump it to Serial
        irReceiver.printRawSignal();

        // output the received bits
        Serial.print("Received ");
        Serial.print(irReceiver.decodedBitCount);
        Serial.print(" bits: ");

        uint8_t bytesWithData = irReceiver.decodedBitCount / 8;
        if (irReceiver.decodedBitCount % 8 > 0)
        {
            bytesWithData++;
        }

        Serial.print("Bytes with data: ");
        Serial.println(bytesWithData);

        printBinaryArray(irReceiver.decodedBits, bytesWithData, " ");

        EventDecodeResult result = eventDecoder.decodeEvent(irReceiver.decodedBitCount, &event);
        if (result == EventDecoded) {
            Serial.print("Decoded event: Team=");
            Serial.print(event.teamID);
            Serial.print(" Player=");
            Serial.print(event.playerID);
            Serial.print(" Event=");
            Serial.print(event.eventID);
            Serial.println(" Data:");
            printBinaryArray(event.data, (event.dataLengthInBits / 8) + 1, " ");

            byte counter = event.data[0];
            int pixel = counter / PIXEL_COUNT;
            if (pixels.getPixelColor(pixel) > 0) {
                pixels.setPixelColor(pixel, 0);
            } else {
                pixels.setPixelColor(pixel, pixels.Color(0, 128, 0));
            }
        }


        // make sure the receiver is reset to decode the next signal
        irReceiver.reset();
    }
}
