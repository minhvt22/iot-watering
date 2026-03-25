# Technical Documentation: Smart Planter Firmware v1.1

This document provides a comprehensive overview of the firmware's architecture, data flows, and module responsibilities.

---

## 1. System Architecture
The firmware follows a **decoupled component architecture** to ensure modularity and maintainability.

### 1.1 Core Components
- **Main (main.cpp)**: Orchestrates the boot sequence, periodic telemetry cycles, and settings synchronization.
- **Provisioning (`Provisioning.h/cpp`)**: Manages WiFi connection via `WiFiManager` and device configuration storage (`Preferences`).
- **SupabaseClient (`SupabaseClient.h/cpp`)**: Handles cloud communication via HTTPS (REST) and WebSockets (Realtime).
- **Hardware (`Hardware.h/cpp`)**: The hardware abstraction layer (HAL) for sensors and actuators.
- **Display (`Display.h/cpp`)**: Manages the OLED UI states and layouts via SPI.
- **Sound (`Sound.h/cpp`)**: High-level melody engine for auditory feedback.

---

## 2. Configuration Management (Config.h)
All system constants are centralized to eliminate magic numbers:
| Category | Constant Examples | Purpose |
| :--- | :--- | :--- |
| **PINS** | `PIN_SOIL_MOISTURE`, `PIN_PUMP_RELAY` | Physical connectivity definitions. |
| **Cloud** | `SUPABASE_URL`, `SUPABASE_KEY` | Backend endpoint authentication. |
| **Logic** | `MAX_PUMP_DURATION_SEC` | Safety thresholds for actuators. |
| **Sync** | `TELEMETRY_INTERVAL_MS` | Data reporting frequency. |

---

## 3. Communication Engine

### 3.1 Telemetry Path (Outbound)
**Protocol**: HTTPS POST (REST)
**Endpoint**: `/rest/v1/telemetry`
**Payload**: JSON containing `device_id`, `moisture`, and `temperature`.
**Smart Logic**:
- **Max Interval**: 30s (Default) to ensure heartbeat in DB.
- **Delta Trigger**: Moisture change > 5.0% or Temp > 1.0°C.
- **Cooldown**: Minimum 5s gap between sends to prevent noise-induced spam.

### 3.2 Remote Control & Config Sync (Inbound)
**Protocol**: WebSockets (Supabase Realtime)
**Channel**: `realtime:device_events` (filtered by `device_id`)
**Dispatcher**: Commands (`pump_on`, `pump_off`) from the `commands` table and Advanced Settings (`auto_water_threshold`, `pump_duration_seconds`) from the `devices` table are parsed and routed to the Hardware layer via `SupabaseClient::handleRealtimeEvent`.

### 3.3 Heartbeat Engine
**Protocol**: Phoenix WebSocket Heartbeat
**Interval**: 30s
**Purpose**: Keeps the WebSocket connection alive through proxies (e.g. Cloudflare) that kill idle TCP connections. Standard raw WS pings are disabled to avoid conflicts.

---

## 4. Provisioning & Pairing Flow (PIN Alignment)

1. **Idle State**: Device boots. If no DeviceID is found, it plays a notify tone and generates a random 6-digit PIN.
2. **PIN Generation**: The device displays the PIN and its MAC address on the OLED.
3. **Realtime Handshake**: The device joins the `realtime:pairing` topic, filtering for its own MAC address.
4. **Web Claiming**: The user enters the MAC and PIN in the Web Dashboard. The app inserts a `device_pairing_sessions` record.
5. **Validation**: The device receives the `INSERT` event, validates the PIN locally, and calls the `claim-device` Edge Function.
6. **Activation**: Backend binds the device to the user. The device stores the `device_id` and restarts.

---

## 5. Safety Failsafes

### 5.1 Actuator Duration Limit
The `Hardware::update()` loop monitors the pump duration. If the pump remains active beyond its dynamic `_pumpDurationLimit` (synced via Supabase, fallback 15s) without a stop command:
- **Action**: The pump is immediately cut (relay off).
- **Feedback**: A high-frequency alarm pulse is played via `Sound::play(ERROR_ALERT)`.
- **Reason**: Prevents water damage in case of cloud latency or sensor failure.

---

## 6. UI & Auditory State Matrix

| State | OLED Display | Sound Pattern |
| :--- | :--- | :--- |
| **Boot** | Splash Screen | Bootup (Rising Tones) |
| **WiFi Setup** | Captive Portal Info | Notification (Short) |
| **Paired** | Live Telemetry Dashboard | Success (Melodic Chirp) |
| **Watering** | "Pump Active" Header | Watering Start/Stop Tones |
| **Error** | Error Details (Inverted) | Alarm (Pulsed) |

---

## 7. Build Environment
- **Platform**: PlatformIO v6.1+
- **Framework**: Arduino
- **Board**: ESP32-WROOM-32 (esp32dev)
- **Key Libraries**: `Adafruit SSD1306`, `Adafruit DHT`, `WebSocketsClient`, `ArduinoJson v7`.
