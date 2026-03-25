#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include <DHT.h>
#include "Config.h"

// --- Notes for Tone (Functional UI sounds) ---
#define NOTE_C5  523
#define NOTE_E5  659
#define NOTE_G5  784
#define NOTE_C6  1047
#define NOTE_A4  440
#define NOTE_F5  698

class Hardware {
public:
  static void setup();
  static void update();

  static float readMoisture();
  static float readTemperature();
  static float readHumidity();

  static void setPump(bool on);
  static bool isPumpOn();

  static void setPumpDurationLimit(int seconds);
  static int getPumpDurationLimit();

  static void setAutoWaterThreshold(int percentage);
  static int getAutoWaterThreshold();

  // Low-level Sound Engine
  static void playTone(unsigned int frequency, unsigned long durationMs);
  static void stopTone();

private:
  static bool _pumpState;
  static unsigned long _pumpStartTime;
  static unsigned long _lastAutoWaterMillis;
  static int _pumpDurationLimit;
  static int _autoWaterThreshold;
  static DHT _dht;
};

#endif
