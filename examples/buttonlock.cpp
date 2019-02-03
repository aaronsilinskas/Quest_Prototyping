
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

const uint32_t BUTTON_PINS[] = {A2, A3, A4, A5};
const uint8_t BUTTON_COUNT = sizeof(BUTTON_PINS) / sizeof(BUTTON_PINS[0]);

const uint8_t PASSWORD_LENGTH = 4;
uint8_t password[PASSWORD_LENGTH];
uint8_t passwordPosition;

bool buttonPressed[BUTTON_COUNT];

enum GameState
{
    WaitingForPlayer,
    ButtonPressed,
    ButtonReleased,
    CorrectButton,
    IncorrectButton,
    Unlocked
};
GameState gameState;

const uint16_t PIXEL_COUNT = 7;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXEL_COUNT, 12, NEO_GRB + NEO_KHZ800);

void setupButtons()
{
    for (uint8_t i = 0; i < BUTTON_COUNT; i++)
    {
        pinMode(BUTTON_PINS[i], INPUT_PULLUP);
        buttonPressed[i] = false;
    }
}

void setupPassword()
{
    randomSeed(analogRead(0));

    Serial.print(F("Generating password:"));
    for (uint8_t i = 0; i < PASSWORD_LENGTH; i++)
    {
        password[i] = random(BUTTON_COUNT);
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

void setup()
{
    setupButtons();
    setupPassword();
    setupLights();

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

bool updateButtonTracking()
{
    bool buttonsChanged = false;
    for (uint8_t i = 0; i < BUTTON_COUNT; i++)
    {
        if (digitalRead(BUTTON_PINS[i]) == 0)
        {
            buttonPressed[i] |= true;
            buttonsChanged = true;
        }
    }
    return buttonsChanged;
}

void resetButtonTracking()
{
    for (uint8_t i = 0; i < BUTTON_COUNT; i++)
    {
        buttonPressed[i] = false;
    }
}

bool anyButtonPressed()
{
    for (uint8_t i = 0; i < BUTTON_COUNT; i++)
    {
        if (buttonPressed[i])
        {
            return true;
        }
    }
    return false;
}

bool allButtonsReleased()
{
    for (uint8_t i = 0; i < BUTTON_COUNT; i++)
    {
        if (digitalRead(BUTTON_PINS[i]) == 0)
        {
            return false;
        }
    }
    return true;
}

void checkButtonAgainstPassword()
{
    uint32_t correctButton = password[passwordPosition];
    bool incorrectButtonPressed = false;
    bool correctButtonPressed = false;

    for (uint8_t button = 0; button < BUTTON_COUNT; button++)
    {
        if (buttonPressed[button])
        {
            Serial.print(F("Pressed: "));
            Serial.println(button);

            correctButtonPressed = (button == correctButton);
            incorrectButtonPressed |= (button != correctButton);
        }
    }

    if (correctButtonPressed && !incorrectButtonPressed)
    {
        updateGameState(CorrectButton);
    }
    else
    {
        updateGameState(IncorrectButton);
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
        updateButtonTracking();
        if (anyButtonPressed())
        {
            updateGameState(ButtonPressed);
        }
        break;
    case ButtonPressed:
        if (allButtonsReleased())
        {
            updateGameState(ButtonReleased);
        }
        break;
    case ButtonReleased:
        checkButtonAgainstPassword();
        break;
    case CorrectButton:
        Serial.println("Yay!");

        resetButtonTracking();

        passwordPosition++;
        updateLightsForProgress();
        if (passwordPosition < PASSWORD_LENGTH)
        {
            updateGameState(WaitingForPlayer);
        }
        else
        {
            updateGameState(Unlocked);
        }

        break;
    case IncorrectButton:
        Serial.println("Boo!");

        resetButtonTracking();

        passwordPosition = 0;

        updateLightsForReset();

        updateGameState(WaitingForPlayer);
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
