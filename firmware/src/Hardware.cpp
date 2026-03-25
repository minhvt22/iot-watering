#include "Hardware.h"

bool Hardware::_pumpState = false;
unsigned long Hardware::_pumpStartTime = 0;
unsigned long Hardware::_lastAutoWaterMillis = 0;
int Hardware::_pumpDurationLimit = MAX_PUMP_DURATION_SEC;
int Hardware::_autoWaterThreshold = 30;
DHT Hardware::_dht(PIN_DHT, DHT11);

void Hardware::setup() {
  pinMode(PIN_PUMP_RELAY, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_PUMP_RELAY, LOW); // Ensure off
  digitalWrite(PIN_BUZZER, LOW);

  _dht.begin();

#ifdef MOCK_HARDWARE
  Serial.println("[Hardware] Running in MOCK mode");
#endif
}

void Hardware::update() {
  unsigned long now = millis();

  // 1. Hardware Failsafe Logic (Highest Priority)
  if (_pumpState && (now - _pumpStartTime) / 1000 >= _pumpDurationLimit) {
    Serial.println("[Hardware] FAILSAFE: Pump duration limit exceeded! Shutting down.");
    setPump(false);
    playTone(1000, 2000); // Alarm
    return;
  }

  // 2. Auto-Watering Guard Clauses
  if (_pumpState) return; // Already watering
  if (_lastAutoWaterMillis != 0 && now - _lastAutoWaterMillis < 600000) return; // Cooldown not met

  float moisture = readMoisture();
  if (moisture <= 0 || moisture >= _autoWaterThreshold) return; // Sensor error or moisture OK

  // 3. Trigger Auto-Watering
  Serial.printf("[Hardware] Auto-Watering Triggered! Moisture %.1f%% < %d%%\n", 
                moisture, _autoWaterThreshold);
  _lastAutoWaterMillis = now;
  setPump(true);
}

float Hardware::readMoisture() {
#ifdef MOCK_HARDWARE
  return 45.5f + (rand() % 10);
#else
  int raw = analogRead(PIN_SOIL_MOISTURE);

  // Guard: Detect sensor disconnection or rail shortcut
  if (raw <= MOISTURE_MIN_RAW || raw >= MOISTURE_MAX_RAW) {
    return SENSOR_ERROR_SENTINEL;
  }

  // Calibration: wetSoilVal=2384, drySoilVal=0
  float percentage = map(raw, SOIL_DRY_VALUE, SOIL_WET_VALUE, 0, 100);
  return constrain(percentage, 0.0f, 100.0f);
#endif
}

float Hardware::readTemperature() {
#ifdef MOCK_HARDWARE
  return 24.0f + (rand() % 5);
#else
  float t = _dht.readTemperature();
  return isnan(t) ? 0.0f : t;
#endif
}

float Hardware::readHumidity() {
#ifdef MOCK_HARDWARE
  return 60.0f + (rand() % 5);
#else
  float h = _dht.readHumidity();
  return isnan(h) ? 0.0f : h;
#endif
}

void Hardware::setPump(bool on) {
  if (on && !_pumpState) {
    _pumpStartTime = millis();
    // Start sound is handled by caller (Sound::play)
  }

  _pumpState = on;
  digitalWrite(PIN_PUMP_RELAY, on ? HIGH : LOW);

  Serial.printf("[Hardware] Pump is now %s\n", on ? "ON" : "OFF");
}

bool Hardware::isPumpOn() { return _pumpState; }

void Hardware::setPumpDurationLimit(int seconds) {
  if (seconds >= 2 && seconds <= 120) {
    _pumpDurationLimit = seconds;
  }
}

int Hardware::getPumpDurationLimit() { return _pumpDurationLimit; }

void Hardware::setAutoWaterThreshold(int percentage) {
  if (percentage >= 0 && percentage <= 100) {
    _autoWaterThreshold = percentage;
  }
}

int Hardware::getAutoWaterThreshold() { return _autoWaterThreshold; }

void Hardware::playTone(unsigned int frequency, unsigned long durationMs) {
#ifdef MOCK_HARDWARE
  Serial.printf("[Hardware] Playing Tone: %d Hz for %d ms\n", frequency,
                durationMs);
#else
  tone(PIN_BUZZER, frequency, durationMs);
#endif
}

void Hardware::stopTone() {
#ifndef MOCK_HARDWARE
  noTone(PIN_BUZZER);
#endif
}
