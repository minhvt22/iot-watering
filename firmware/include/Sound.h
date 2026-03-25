#ifndef SOUND_H
#define SOUND_H

#include <Arduino.h>

enum class SoundEvent {
  BOOT_UP,
  WIFI_CONNECTED,
  WATERING_START,
  WATERING_STOP,
  SUCCESS,
  ERROR_ALERT,
  NOTIFICATION
};

class Sound {
public:
  static void play(SoundEvent event);
  static void
  update(); // For async patterns (not needed for simple blocking play)
};

#endif
