#include "Sound.h"
#include "Hardware.h"

void Sound::play(SoundEvent event) {
  switch (event) {
  case SoundEvent::BOOT_UP:
    Hardware::playTone(NOTE_C5, 100);
    delay(50);
    Hardware::playTone(NOTE_E5, 100);
    delay(50);
    Hardware::playTone(NOTE_G5, 100);
    delay(50);
    Hardware::playTone(NOTE_C6, 200);
    break;

  case SoundEvent::SUCCESS:
    Hardware::playTone(NOTE_G5, 100);
    delay(50);
    Hardware::playTone(NOTE_C6, 200);
    break;

  case SoundEvent::WIFI_CONNECTED:
    Hardware::playTone(NOTE_A4, 100);
    delay(50);
    Hardware::playTone(NOTE_C5, 200);
    break;

  case SoundEvent::ERROR_ALERT:
    for (int i = 0; i < 3; i++) {
      Hardware::playTone(1000, 100);
      delay(100);
    }
    break;

  case SoundEvent::WATERING_START:
    Hardware::playTone(NOTE_F5, 100);
    delay(50);
    Hardware::playTone(NOTE_A4, 100);
    break;

  case SoundEvent::WATERING_STOP:
    Hardware::playTone(NOTE_A4, 100);
    delay(50);
    Hardware::playTone(NOTE_F5, 100);
    break;

  case SoundEvent::NOTIFICATION:
    Hardware::playTone(NOTE_C6, 100);
    delay(50);
    Hardware::playTone(NOTE_G5, 100);
    break;
  }
}
