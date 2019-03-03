#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_FreeTouch.h>
#include <Quest_ComboLock.h>
#include <Quest_EventEncoder.h>
#include <Quest_IR_Transmitter.h>

const uint32_t BUTTON_PINS[] = {A1, A2, A3, A4, A5};
const uint8_t BUTTON_COUNT = sizeof(BUTTON_PINS) / sizeof(BUTTON_PINS[0]);
const uint16_t TOUCH_THRESHOLD = 150;
Adafruit_FreeTouch qts[BUTTON_COUNT];
uint16_t restingButtonMeasurement[BUTTON_COUNT];

const uint8_t EVENT_QUEUE_SIZE = 10;
Event eventQueueBuffer[EVENT_QUEUE_SIZE];
Quest_EventQueue eventQueue = Quest_EventQueue(eventQueueBuffer, EVENT_QUEUE_SIZE, 0, 0);

const uint8_t PASSWORD_LENGTH = 4;
uint16_t password[PASSWORD_LENGTH];
Quest_ComboLock lock = Quest_ComboLock(password, PASSWORD_LENGTH, &eventQueue);

const uint16_t PIXEL_COUNT = 7;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXEL_COUNT, 12, NEO_GRB + NEO_KHZ800);

Quest_IR_Transmitter irTransmitter = Quest_IR_Transmitter();
Quest_EventEncoder eventEncoder = Quest_EventEncoder(irTransmitter.encodedBits, QIR_BUFFER_SIZE);

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
}

void setupLock()
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
    setupLock();
    setupLights();
}

uint16_t readButtonState()
{
    uint16_t buttonState = 0;
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

    return buttonState;
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

void updateLightsForProgress(uint8_t position, uint8_t length)
{
    uint16_t pixelsToLight = PIXEL_COUNT * position / length;
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

void updateLightsForReset()
{
    for (uint16_t blinkTimes = 0; blinkTimes < 3; blinkTimes++)
    {
        fadeInPixels(500, 0, PIXEL_COUNT, 255, 0, 0);
    }
    pixels.clear();
    pixels.show();
}

void updateLightsForUnlock()
{
    for (uint16_t i = 0; i < PIXEL_COUNT; i++)
    {
        pixels.setPixelColor(i, 128, 128, 128);
        pixels.show();
        delay(500);
        pixels.setPixelColor(i, 0, 0, 0);
    }
    fadeInPixels(2000, 0, PIXEL_COUNT, 255, 255, 255);
}

void sendIREvent(Event *e)
{
    if (eventEncoder.encodeToBuffer(e) == EventEncoded)
    {
        irTransmitter.sendBits(eventEncoder.encodedBitCount);
    }
}

void loop()
{
    if (lock.unlocked)
    {
        // wait for a bit then lock it
        delay(2000);
        lock.lock();
    }
    else
    {
        uint16_t buttonState = readButtonState();
        if (buttonState != 0)
        {
            lock.tryStep(buttonState);
        }
    }

    Event next;
    while (eventQueue.poll(&next))
    {
        sendIREvent(&next);

        if (next.eventID == QE_ID_PROGRESS)
        {
            if (next.data[0] == 0)
            {
                updateLightsForReset();
            }
            else
            {
                updateLightsForProgress(next.data[0], next.data[1]);
            }
        }
        else if (next.eventID == QE_ID_UNLOCKED)
        {
            updateLightsForUnlock();
        }
        else if (next.eventID == QE_ID_LOCKED)
        {
            pixels.clear();
            pixels.show();
        }
    }
}
