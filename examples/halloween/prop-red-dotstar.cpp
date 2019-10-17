/* prop.cpp Example sketch for remote Prop FX
 * Using DFPlayer MP3 player and FastLED
 *
 */

#include <Quest_IR_Receiver.h>
#include <Quest_Trigger.h>
#include "Quest_EventDecoder.h"
#include <FastLED.h>
#include <DFRobotDFPlayerMini.h>


// NOTE: CHANGE THIS FOR EACH PROP OR SHARE ID FOR MULTI-TRIGGER
const byte ID_PLAYER = 1;

const int PIN_MANUAL_TRIGGER = A0;
Quest_Trigger manualTrigger = Quest_Trigger(PIN_MANUAL_TRIGGER, false, 50);

const int PIN_IR_RECEIVER = 7;
Quest_IR_Receiver irReceiver;
Quest_EventDecoder eventDecoder = Quest_EventDecoder(irReceiver.decodedBits, QIR_BUFFER_SIZE);
Event event;

DFRobotDFPlayerMini soundPlayer;

#define PIN_NEOPIXELS 5
#define PIN_DOTSTAR_CLK 12
#define PIN_DOTSTAR_DATA 11
#define LED_COUNT 30
CRGB leds[LED_COUNT];
CRGB dotstar[1];

CRGBPalette16 ledsPalette;
bool ledsReverseDirection;
#define LED_BRIGHTNESS 128
#define LED_FRAMES_PER_SECOND 60
#define LED_MINIMUM_FRAME_DELAY 1000 / LED_FRAMES_PER_SECOND

bool playingEffect = false;

void setup()
{
    Serial.begin(9600);

    // set up manual trigger
    pinMode(PIN_MANUAL_TRIGGER, INPUT_PULLUP);

    // set up IR
    irReceiver.begin(PIN_IR_RECEIVER);

    // set up LEDs
    //FastLED.addLeds<NEOPIXEL, PIN_NEOPIXELS>(leds, LED_COUNT);
    FastLED.addLeds<DOTSTAR, PIN_DOTSTAR_DATA, PIN_DOTSTAR_CLK, BGR>(leds, LED_COUNT);

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
    } else {
        Serial.println(F("DFPlayer Mini ready."));
    }
}

void playEerieGreenGlow()
{
    Serial.println("Starting eerie green glow...");
    soundPlayer.volume(30);
    soundPlayer.playFolder(1, 1);

    //ledsPalette = CRGBPalette16(CRGB::Black, CRGB::Green, CRGB::LawnGreen, CRGB::DarkSeaGreen); // green glow
    ledsPalette = CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::DarkSalmon, CRGB::DarkViolet); // red glow
    //ledsPalette = ForestColors_p;
    //ledsPalette = OceanColors_p;
    //ledsPalette = HeatColors_p;
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
        playEerieGreenGlow();
    }
}

void updateLEDsFromPalette()
{
    static uint16_t sPseudotime = 0;
    static uint16_t sLastMillis = 0;
    static uint16_t sHue16 = 0;

    uint8_t brightdepth = beatsin88(341, 96, 224);
    uint16_t brightnessthetainc16 = beatsin88(203, (25 * 256), (40 * 256));
    uint8_t msmultiplier = beatsin88(147, 23, 60);

    uint16_t hue16 = sHue16; //gHue * 256;
    uint16_t hueinc16 = beatsin88(113, 300, 1500);

    uint16_t ms = millis();
    uint16_t deltams = ms - sLastMillis;
    sLastMillis = ms;
    sPseudotime += deltams * msmultiplier;
    sHue16 += deltams * beatsin88(400, 5, 9);
    uint16_t brightnesstheta16 = sPseudotime;

    for (uint16_t i = 0; i < LED_COUNT; i++)
    {
        hue16 += hueinc16;
        uint8_t hue8 = hue16 / 256;
        uint16_t h16_128 = hue16 >> 7;
        if (h16_128 & 0x100)
        {
            hue8 = 255 - (h16_128 >> 1);
        }
        else
        {
            hue8 = h16_128 >> 1;
        }

        brightnesstheta16 += brightnessthetainc16;
        uint16_t b16 = sin16(brightnesstheta16) + 32768;

        uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
        uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
        bri8 += (255 - brightdepth);

        uint8_t index = hue8;
        //index = triwave8( index);
        index = scale8(index, 240);

        CRGB newcolor = ColorFromPalette(ledsPalette, index, bri8);

        uint16_t pixelnumber = i;
        pixelnumber = (LED_COUNT - 1) - pixelnumber;

        nblend(leds[pixelnumber], newcolor, 128);
    }
}

void loop()
{
    if (playingEffect)
    {
        updateLEDsFromPalette();

        FastLED.show();
        FastLED.delay(LED_MINIMUM_FRAME_DELAY);
    }

    if (manualTrigger.isTriggered())
    {
        togglePlayingEffect();
        manualTrigger.delayNextTrigger(250);
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
