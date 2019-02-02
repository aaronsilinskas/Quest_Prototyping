
#include <Arduino.h>

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
    randomSeed(micros());

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

void setup()
{
    setupButtons();
    setupPassword();

    gameState = WaitingForPlayer;
}

void updateGameState(GameState newState)
{
    // Serial.print(F("State change from "));
    // Serial.print(gameState);
    // Serial.print(F(" to "));
    // Serial.println(newState);

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
        updateGameState(WaitingForPlayer);
        break;
    case Unlocked:
        Serial.println("UNLOCKED!");

        passwordPosition = 0;
        updateGameState(WaitingForPlayer);
        break;
    }
}
