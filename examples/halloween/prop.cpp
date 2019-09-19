/* prop.cpp Example sketch for remote Prop FX
 * Using DFPlayer MP3 player and FastLED
 *
 */

#include <Quest_IR_Receiver.h>
#include "Quest_EventDecoder.h"
#include <FastLED.h>
#include <DFRobotDFPlayerMini.h>

// NOTE: CHANGE THIS FOR EACH PROP OR SHARE ID FOR MULTI-TRIGGER
const byte ID_PLAYER = 1;

const int PIN_MANUAL_TRIGGER = A0;

const int PIN_IR_RECEIVER = 7;
Quest_IR_Receiver irReceiver;
Quest_EventDecoder eventDecoder = Quest_EventDecoder(irReceiver.decodedBits, QIR_BUFFER_SIZE);
Event event;

DFRobotDFPlayerMini soundPlayer;

#define PIN_PIXELS 10
#define PIXEL_COUNT 12

void setup()
{
    Serial.begin(9600);

    // set up manual trigger
    pinMode(PIN_MANUAL_TRIGGER, INPUT_PULLUP);

    // set up IR
    irReceiver.begin(PIN_IR_RECEIVER);

    // set up MP3 player
    Serial1.begin(9600);
    if (!soundPlayer.begin(Serial1))
    {
        Serial.println(F("Unable to begin:"));
        Serial.println(F("1.Please recheck the connection!"));
        Serial.println(F("2.Please insert the SD card!"));
        while (true)
            ;
    }
    Serial.println(F("DFPlayer Mini ready."));
}

bool isManuallyTriggered() {
  static long lastPress = millis();
  static int lastState = false;

  if (millis() - lastPress < 250) {
      return false;
  }

  int currentState = digitalRead(PIN_MANUAL_TRIGGER);
  if (currentState == 0 && lastState == 1) {
      lastPress = millis();
      lastState = 0;
      return true;
  } else if (currentState == 1) {
      lastState = 1;
      return false;
  }
}

void playEerieGreenGlow()
{
    Serial.println("Starting eerie green glow...");
    soundPlayer.volume(30);
    soundPlayer.playFolder(1, 1);
}

void fadeOutEffect()
{
    Serial.println("Fading out...");
    soundPlayer.stop();
}

void togglePlayingEffect()
{
    static bool playingEffect = false;

    if (playingEffect)
    {
        fadeOutEffect();
        playingEffect = false;
    }
    else
    {
        playingEffect = true;
        playEerieGreenGlow();
    }
}

void loop()
{

    if (isManuallyTriggered())
    {
        togglePlayingEffect();
    }

    // check to see if the receiver decoded a signal
    if (irReceiver.hasSignal())
    {
        EventDecodeResult result = eventDecoder.decodeEvent(irReceiver.decodedBitCount, &event);
        if (result == EventDecoded)
        {
            Serial.print("Decoded event: Team=");
            Serial.print(event.teamID);
            Serial.print(" Player=");
            Serial.print(event.playerID);
            Serial.print(" Event=");
            Serial.print(event.eventID);
            Serial.println(" Data:");
            printBinaryArray(event.data, (event.dataLengthInBits / 8) + 1, " ");

            if (event.teamID == 0 && event.playerID == ID_PLAYER)
            {
                togglePlayingEffect();
            }
        }

        // make sure the receiver is reset to decode the next signal
        irReceiver.reset();
    }
}
