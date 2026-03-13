#include <Arduino.h>
#include "Hardware.h"
#include "Provisioning.h"
#include "SupabaseClient.h"
#include "env.h"

unsigned long lastTelemetry = 0;
const unsigned long TELEMETRY_INTERVAL = 30 * 60 * 1000; // 30 minutes

void setup() {
    Serial.begin(115200);
    Serial.println("\n--- Smart IoT Water System Booting ---");

    Hardware::setup();
    Provisioning::setup();
    SupabaseClient::setup(SUPABASE_URL, SUPABASE_KEY);

    // Initial Registration Flow
    if (!Provisioning::isConfigured()) {
        String claimToken = Provisioning::getClaimToken();
        if (claimToken.length() > 0) {
            Serial.println("[Main] Attempting device registration...");
            if (SupabaseClient::registerDevice(claimToken)) {
                Serial.println("[Main] Registration Successful!");
            } else {
                Serial.println("[Main] Registration Failed. Will retry later or re-config needed.");
            }
        } else {
            Serial.println("[Main] No device ID and no claim token. Waiting for config portal.");
        }
    }

    if (Provisioning::isConfigured()) {
        SupabaseClient::syncSettings();
        // Send initial telemetry on boot
        SupabaseClient::sendTelemetry(Hardware::readMoisture(), Hardware::readTemperature());
    }
}

void loop() {
    Hardware::update();
    SupabaseClient::update();

    // Telemetry Reporting Loop
    if (Provisioning::isConfigured()) {
        if (millis() - lastTelemetry >= TELEMETRY_INTERVAL) {
            lastTelemetry = millis();
            float moisture = Hardware::readMoisture();
            float temp = Hardware::readTemperature();
            
            Serial.printf("[Main] Reporting Telemetry: M=%.2f, T=%.2f\n", moisture, temp);
            SupabaseClient::sendTelemetry(moisture, temp);
        }
    }

    // Handle Manual Reset (Holding button on boot or specific GPIO)
    // if (digitalRead(0) == LOW) Provisioning::reset(); 

    delay(10); // Small yield for RTOS
}
