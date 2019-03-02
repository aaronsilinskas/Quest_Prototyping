#include <Arduino.h>
#include <unity.h>

#include "Quest_ComboLock.h"

#define KEY_MAX_LENGTH 10
uint16_t key[KEY_MAX_LENGTH];

void setupTests()
{
    randomSeed(analogRead(0));
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
    Quest_ComboLock cl = Quest_ComboLock(key, KEY_MAX_LENGTH);
    for (uint8_t step = 0; step < KEY_MAX_LENGTH; step++)
    {
        uint16_t validStep = key[step];
        TEST_ASSERT_TRUE(cl.tryStep(validStep));
        TEST_ASSERT_EQUAL(step + 1, cl.keyPosition);
    }
}

void test_invalid_step_resets_key()
{
    Quest_ComboLock cl = Quest_ComboLock(key, KEY_MAX_LENGTH);
    uint8_t stepToChooseInvalidValue = random(KEY_MAX_LENGTH);

    // choose valid values until an invalid value will be tests
    for (uint8_t step = 0; step < stepToChooseInvalidValue; step++)
    {
        uint16_t validStep = key[step];
        TEST_ASSERT_TRUE(cl.tryStep(validStep));
    }

    // choose an invalid value
    uint16_t invalidStep = !(key[stepToChooseInvalidValue]);
    TEST_ASSERT_FALSE(cl.tryStep(invalidStep));
    TEST_ASSERT_EQUAL(0, cl.keyPosition);
}

void test_all_valid_steps_unlock_key()
{
    Quest_ComboLock cl = Quest_ComboLock(key, KEY_MAX_LENGTH);
    // try valid values for all but the last step
    for (uint8_t step = 0; step < KEY_MAX_LENGTH - 1; step++)
    {
        cl.tryStep(key[step]);
        TEST_ASSERT_FALSE(cl.unlocked);
    }
    // try last step
    cl.tryStep(key[KEY_MAX_LENGTH - 1]);
    TEST_ASSERT_TRUE(cl.unlocked);
}

// key is locked until key position = key length
// key length is available for percentage progress
// tryStep after last fails and resets

void setup()
{
    delay(4000);
    setupTests();

    UNITY_BEGIN();

    RUN_TEST(test_locked_by_default);
    RUN_TEST(test_valid_steps_progress_key);
    RUN_TEST(test_invalid_step_resets_key);
    RUN_TEST(test_all_valid_steps_unlock_key);

    UNITY_END();
}

void loop()
{
}
