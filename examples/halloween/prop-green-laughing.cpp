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
#define PIN_SOUND_PLAYING 9
#define SOUND_FOLDER_NUMBER 1
#define SOUND_FILE_NUMBER 2

#define PIN_NEOPIXELS 5
#define PIN_DOTSTAR_CLK 12
#define PIN_DOTSTAR_DATA 11
#define LED_COUNT 12
CRGB leds[LED_COUNT];
CRGB dotstar[1];

CRGBPalette16 ledsPalette;
bool ledsReverseDirection;
#define LED_FADE_SPEED 12
#define LED_BRIGHTNESS 128
#define LED_FRAMES_PER_SECOND 120
#define LED_MINIMUM_FRAME_DELAY 1000 / LED_FRAMES_PER_SECOND
byte ledColorIndex[LED_COUNT];
int fadeAmount = 0;
int brightness = 0;

bool playingEffect = false;
unsigned long effectStartTimeMs = 0;

void setup()
{
    Serial.begin(9600);

    // set up manual trigger
    pinMode(PIN_MANUAL_TRIGGER, INPUT_PULLUP);

    // set up IR
    irReceiver.begin(PIN_IR_RECEIVER);

    // set up LEDs
    FastLED.addLeds<NEOPIXEL, PIN_NEOPIXELS>(leds, LED_COUNT);
    //FastLED.addLeds<DOTSTAR, PIN_DOTSTAR_DATA, PIN_DOTSTAR_CLK, BGR>(leds, LED_COUNT);

    FastLED.setCorrection(TypicalLEDStrip);
    FastLED.setDither(1);
    FastLED.setBrightness(LED_BRIGHTNESS);

    // turn off dotstar
    FastLED.addLeds<DOTSTAR, 8, 6, GBR>(dotstar, 1);
    dotstar[0] = CRGB::Black;

    FastLED.show();

    // set up MP3 player
    Serial1.begin(9600);
    if (!soundPlayer.begin(Serial1))
    {
        Serial.println(F("Unable to begin:"));
        Serial.println(F("1.Please recheck the connection!"));
        Serial.println(F("2.Please insert the SD card!"));
        dotstar[0] = CRGB::Red;
        FastLED.show();
    }
    else
    {
        Serial.println(F("DFPlayer Mini ready."));
    }
    pinMode(PIN_SOUND_PLAYING, INPUT_PULLUP);
}

bool isManuallyTriggered()
{
    static long lastPress = millis();
    static int lastState = false;

    if (millis() - lastPress < 250)
    {
        return false;
    }

    int currentState = digitalRead(PIN_MANUAL_TRIGGER);
    if (currentState == 0 && lastState == 1)
    {
        lastPress = millis();
        lastState = 0;
        return true;
    }
    else if (currentState == 1)
    {
        lastState = 1;
        return false;
    }
    return false;
}

void startEffect()
{
    Serial.println("Starting effect...");
    soundPlayer.volume(30);
    soundPlayer.playFolder(SOUND_FOLDER_NUMBER, SOUND_FILE_NUMBER);

    ledsPalette = CRGBPalette16(CRGB::Black, CRGB::Green, CRGB::LawnGreen, CRGB::DarkSeaGreen);
    //ledsPalette = CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::DarkSalmon, CRGB::DarkViolet); // red glow
    //ledsPalette = ForestColors_p;
    //ledsPalette = OceanColors_p;
    //ledsPalette = HeatColors_p;
    effectStartTimeMs = millis();
    brightness = 0;
    fadeAmount = LED_FADE_SPEED;
    for (uint16_t i = 0; i < LED_COUNT; i++)
    {
        ledColorIndex[i] = random(128);
    }
}

void fadeOutEffect()
{
    Serial.println("Fading out...");
    soundPlayer.stop();

    FastLED.clear();
    FastLED.show();
}

void togglePlayingEffect()
{
    if (playingEffect)
    {
        playingEffect = false;
        fadeOutEffect();
    }
    else
    {
        playingEffect = true;
        startEffect();
    }
}

void updateLEDsFromPalette()
{
    brightness += fadeAmount;
    if (brightness > LED_BRIGHTNESS)
    {
        brightness = LED_BRIGHTNESS;
        fadeAmount = -LED_FADE_SPEED;
    }
    else if (brightness <= 0)
    {
        fadeAmount = LED_FADE_SPEED;
        brightness = 0;
        for (uint16_t i = 0; i < LED_COUNT; i++)
        {
            ledColorIndex[i] = random(128);
        }
    }
    for (uint16_t i = 0; i < LED_COUNT; i++)
    {
        ledColorIndex[i] = ledColorIndex[i] + 1;
        if (ledColorIndex[i] > 128) {
            ledColorIndex[i] = 0;
        }
        leds[i] = ColorFromPalette(ledsPalette, ledColorIndex[i], brightness);
    }
}

void loop()
{
    if (playingEffect)
    {
        updateLEDsFromPalette();

        FastLED.show();
        FastLED.delay(LED_MINIMUM_FRAME_DELAY);

        if (millis() > effectStartTimeMs + 1000 && digitalRead(PIN_SOUND_PLAYING) == HIGH)
        {
            togglePlayingEffect();
        }
    }

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
