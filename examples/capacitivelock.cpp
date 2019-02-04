
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_FreeTouch.h>
#include <Quest_Audio.h>

const uint32_t BUTTON_PINS[] = {A1, A2, A3, A4, A5};
const uint8_t BUTTON_COUNT = sizeof(BUTTON_PINS) / sizeof(BUTTON_PINS[0]);
const uint16_t TOUCH_THRESHOLD = 150;
Adafruit_FreeTouch qts[BUTTON_COUNT];
uint16_t restingButtonMeasurement[BUTTON_COUNT];
uint16_t buttonState;

const uint8_t PASSWORD_LENGTH = 4;
uint8_t password[PASSWORD_LENGTH];
uint8_t passwordPosition;

enum GameState
{
    WaitingForPlayer,
    ButtonPressed,
    CorrectButton,
    IncorrectButton,
    WaitingForRelease,
    Unlocked
};
GameState gameState;

const uint16_t PIXEL_COUNT = 7;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXEL_COUNT, 12, NEO_GRB + NEO_KHZ800);

Quest_Audio audio = Quest_Audio();
const uint8_t SD_CS_PIN = 10;
const char *SFX_CORRECT_BUTTON = "WAND-S~1.WAV";

void setupButtons()
{
    for (uint8_t i = 0; i < BUTTON_COUNT; i++)
    {
        qts[i] = Adafruit_FreeTouch(BUTTON_PINS[i], OVERSAMPLE_4, RESISTOR_50K, FREQ_MODE_NONE);
        if (!qts[i].begin())
        {
            Serial.print(F("ERROR: Could not start capacitive touch on pin: "));
            Serial.println(BUTTON_PINS[i]);
        }
        else
        {
            restingButtonMeasurement[i] = qts[i].measure();
        }
    }
    buttonState = 0;
}

void setupPassword()
{
    randomSeed(analogRead(0));

    Serial.print(F("Generating password:"));
    for (uint8_t i = 0; i < PASSWORD_LENGTH; i++)
    {
        password[i] = (1 << random(BUTTON_COUNT));
        Serial.print(F(" "));
        Serial.print(password[i]);
    }
    Serial.println();

    passwordPosition = 0;
}

void setupLights()
{
    pixels.begin();
    pixels.setBrightness(32);
    pixels.clear();
    pixels.show();
}

void setupSound()
{
    audio.begin(SD_CS_PIN);
}

void setup()
{
    setupButtons();
    setupPassword();
    setupLights();
    setupSound();

    gameState = WaitingForPlayer;
}

void updateGameState(GameState newState)
{
    Serial.print(F("State change from "));
    Serial.print(gameState);
    Serial.print(F(" to "));
    Serial.println(newState);

    gameState = newState;
}

void updateButtonState()
{
    buttonState = 0;
    for (uint8_t i = 0; i < BUTTON_COUNT; i++)
    {
        buttonState <<= 1;
        uint16_t currentButtonState = qts[i].measure();

        // Serial.print(F("Button "));
        // Serial.print(i);
        // Serial.print(F(" Resting "));
        // Serial.print(restingButtonMeasurement[i]);
        // Serial.print(F(" Current "));
        // Serial.println(currentButtonState);

        if (currentButtonState - restingButtonMeasurement[i] > TOUCH_THRESHOLD)
        {
            buttonState |= 1;
        }
    }

    if (buttonState > 0)
    {
        Serial.print(F("Button state: "));
        Serial.println(buttonState, BIN);
    }
}

void fadeInPixels(uint64_t transitionTime, uint16_t firstPixel, uint16_t pixelCount, uint8_t rT, uint8_t gT, uint8_t bT)
{
    uint64_t currentTime = millis();
    uint64_t endTime = currentTime + transitionTime;
    while (currentTime < endTime)
    {
        uint64_t ellapsedTime = transitionTime - (endTime - currentTime);
        for (uint16_t i = firstPixel; i < firstPixel + pixelCount; i++)
        {
            uint16_t r = rT * ellapsedTime / transitionTime;
            uint16_t g = gT * ellapsedTime / transitionTime;
            uint16_t b = bT * ellapsedTime / transitionTime;
            pixels.setPixelColor(i, r, g, b);
        }
        pixels.show();
        delay(50);
        currentTime = millis();
    }
}

void updateLightsForProgress()
{
    if (passwordPosition < PASSWORD_LENGTH)
    {
        uint16_t pixelsToLight = PIXEL_COUNT * passwordPosition / PASSWORD_LENGTH;
        uint16_t firstPixel = 0;
        for (uint16_t i = 0; i < pixelsToLight; i++)
        {
            if (pixels.getPixelColor(i) != 0)
            {
                firstPixel++;
            }
        }
        fadeInPixels(1000, firstPixel, pixelsToLight - firstPixel, 0, 255, 32);
    }
    else
    {
        fadeInPixels(2000, 0, PIXEL_COUNT, 255, 255, 255);
    }
    pixels.show();
}

void updateLightsForReset()
{
    for (uint16_t blinkTimes = 0; blinkTimes < 3; blinkTimes++)
    {
        fadeInPixels(500, 0, PIXEL_COUNT, 255, 0, 0);
    }
    pixels.clear();
    pixels.show();
}

void loop()
{
    switch (gameState)
    {
    case WaitingForPlayer:
        updateButtonState();
        if (buttonState != 0)
        {
            updateGameState(ButtonPressed);
        }

        break;
    case ButtonPressed:
        Serial.print(F("Expected button state: "));
        Serial.println(password[passwordPosition], BIN);
        Serial.print(F("Actual state: "));
        Serial.println(buttonState, BIN);

        if (password[passwordPosition] == buttonState)
        {
            updateGameState(CorrectButton);
        }
        else
        {
            updateGameState(IncorrectButton);
        }
        break;
    case CorrectButton:
        Serial.println("Correct Button!");

        passwordPosition++;
        updateLightsForProgress();

        audio.playSound(SFX_CORRECT_BUTTON, 128);

        if (passwordPosition < PASSWORD_LENGTH)
        {
            updateGameState(WaitingForRelease);
        }
        else
        {
            updateGameState(Unlocked);
        }

        break;
    case IncorrectButton:
        Serial.println("Incorrect Button!");

        passwordPosition = 0;

        updateLightsForReset();

        updateGameState(WaitingForRelease);
        break;
    case WaitingForRelease:
        updateButtonState();
        if (buttonState == 0)
        {
            updateGameState(WaitingForPlayer);
        }
        break;
    case Unlocked:
        Serial.println("UNLOCKED!");

        passwordPosition = 0;
        delay(2000);
        pixels.clear();
        pixels.show();

        updateGameState(WaitingForPlayer);
        break;
    }
}
