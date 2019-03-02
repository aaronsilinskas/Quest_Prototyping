#include <Arduino.h>
#include <unity.h>

#include "Quest_ComboLock.h"

#define KEY_MAX_LENGTH 10
uint16_t key[KEY_MAX_LENGTH];

void randomizeKey()
{
    for (uint8_t i = 0; i < KEY_MAX_LENGTH; i++)
    {
        key[i] = random(0xFFFF);
    }
}

void test_locked_by_default()
{
    Quest_ComboLock cl = Quest_ComboLock(key, KEY_MAX_LENGTH);
    TEST_ASSERT_FALSE(cl.unlocked);
}

void test_valid_steps_progress_key()
{
    randomizeKey();
    Quest_ComboLock cl = Quest_ComboLock(key, KEY_MAX_LENGTH);
    // enter valid steps except the last one
    for (uint8_t step = 0; step < KEY_MAX_LENGTH; step++)
    {
        uint16_t validStep = key[step];
        TEST_ASSERT_TRUE(cl.tryStep(validStep));
        TEST_ASSERT_EQUAL(step + 1, cl.keyPosition);
    }
}

// key is locked until key position = key length
// key length is available for percentage progress

void setup()
{
    delay(4000);
    randomSeed(analogRead(0));

    UNITY_BEGIN();

    RUN_TEST(test_locked_by_default);
    RUN_TEST(test_valid_steps_progress_key);

    UNITY_END();
}

void loop()
{
}
