#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- .env Settings ---
#include "env.h"

// --- Hardware Pins (ESP32) ---
#define PIN_SOIL_MOISTURE 34
#define PIN_DHT 16
#define PIN_PUMP_RELAY 26
#define PIN_BUZZER 25

// --- OLED SPI Pins ---
#define OLED_MOSI 19
#define OLED_CLK 18
#define OLED_DC 22
#define OLED_CS 23
#define OLED_RST 21

// --- Hardware Calibration & Offsets ---
#define SOIL_WET_VALUE 2384
#define SOIL_DRY_VALUE 0
#define MAX_PUMP_DURATION_SEC 15
#define MOISTURE_MIN_RAW 10
#define MOISTURE_MAX_RAW 4080
#define SENSOR_ERROR_SENTINEL -1.0f

// --- Logic & Intervals (ms) ---
#define TELEMETRY_INTERVAL_MS 30000
#define SETTINGS_SYNC_INTERVAL_MS 60000
#define RECONNECT_INTERVAL_MS 5000

// --- Provisioning ---
#define PREFS_NAMESPACE "water-sys"
#define CAPTIVE_PORTAL_SSID "SmartPlanter-Setup"
#define MDNS_NAME "water-sys"
#define PIN_MIN_RANGE 100000
#define PIN_MAX_RANGE 999999
#define PAIRING_TIMEOUT_MS 900000 // 15 Minutes (Matches DB TTL)

#endif
