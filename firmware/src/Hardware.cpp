#include "Hardware.h"

bool Hardware::_pumpState = false;
unsigned long Hardware::_pumpStartTime = 0;
int Hardware::_pumpDurationLimit = MAX_PUMP_DURATION_DEFAULT;

void Hardware::setup() {
    pinMode(PIN_PUMP_RELAY, OUTPUT);
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_PUMP_RELAY, LOW); // Ensure off
    
    #ifdef MOCK_HARDWARE
    Serial.println("[Hardware] Running in MOCK mode");
    #endif
}

void Hardware::update() {
    // Hardware Failsafe Logic
    if (_pumpState) {
        unsigned long elapsed = (millis() - _pumpStartTime) / 1000;
        if (elapsed >= _pumpDurationLimit) {
            Serial.println("[Hardware] FAILSAFE: Pump duration limit exceeded! Shutting down.");
            setPump(false);
            // In a real app, we might also set an error flag to report to Supabase
        }
    }
}

float Hardware::readMoisture() {
    #ifdef MOCK_HARDWARE
    return 45.5f + (rand() % 10);
    #else
    int raw = analogRead(PIN_SOIL_MOISTURE);
    // Rough calibration: 4095 is dry, ~1500 is wet for some resistive sensors
    // Map to 0-100% (Inverted since lower ADC = wetter)
    float percentage = map(raw, 4095, 1000, 0, 100);
    return constrain(percentage, 0.0f, 100.0f);
    #endif
}

float Hardware::readTemperature() {
    #ifdef MOCK_HARDWARE
    return 24.0f + (rand() % 5);
    #else
    // Placeholder for actual sensor lib
    return 0.0f; 
    #endif
}

void Hardware::setPump(bool on) {
    if (on && !_pumpState) {
        _pumpStartTime = millis();
    }
    
    _pumpState = on;
    digitalWrite(PIN_PUMP_RELAY, on ? HIGH : LOW);
    digitalWrite(PIN_STATUS_LED, on ? HIGH : LOW); // Visual indicator
    
    Serial.printf("[Hardware] Pump is now %s\n", on ? "ON" : "OFF");
}

bool Hardware::isPumpOn() {
    return _pumpState;
}

void Hardware::setPumpDurationLimit(int seconds) {
    if (seconds >= 5 && seconds <= 120) {
        _pumpDurationLimit = seconds;
    }
}

int Hardware::getPumpDurationLimit() {
    return _pumpDurationLimit;
}
