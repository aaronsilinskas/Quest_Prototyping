#ifndef mw_game_h
#define mw_game_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Quest_Event.h>
#include <Quest_EventQueue.h>

enum Difficulty {
  Beginner,
  Intermediate,
  Advanced,
  Expert
};

class MW_Game
{
public:
  MW_Game(Quest_EventQueue &eventQueue);
  virtual void update(uint64_t ellapsedTime);
  virtual void saveState(const JsonObject &dest);
  virtual void loadState(const JsonObject &src);

protected:
  Quest_EventQueue &eventQueue;
};

#endif
