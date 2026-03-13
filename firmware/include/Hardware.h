#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>

// --- Pin Definitions ---
#define PIN_SOIL_MOISTURE 34  // ADC1_6
#define PIN_TEMP_SENSOR   32  // (Optional OneWire/DHT)
#define PIN_PUMP_RELAY    26
#define PIN_STATUS_LED    2

// --- Failsafe Constants ---
#define MAX_PUMP_DURATION_DEFAULT 15 // Seconds

class Hardware {
public:
    static void setup();
    static void update();

    static float readMoisture();
    static float readTemperature();
    
    static void setPump(bool on);
    static bool isPumpOn();

    static void setPumpDurationLimit(int seconds);
    static int getPumpDurationLimit();

private:
    static bool _pumpState;
    static unsigned long _pumpStartTime;
    static int _pumpDurationLimit;
};

#endif
