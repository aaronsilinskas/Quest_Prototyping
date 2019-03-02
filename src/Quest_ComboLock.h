#ifndef quest_combolock_h
#define quest_combolock_h

#include <Arduino.h>

class Quest_ComboLock
{
public:
  bool unlocked = false;
  uint8_t keyPosition;

  Quest_ComboLock(uint16_t *key, uint8_t keyLength);
  bool tryStep(uint16_t value);

private:
  uint16_t *key;
  uint8_t keyLength;
};

#endif
