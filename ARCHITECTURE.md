# System Architecture: Smart IoT Water System

This document outlines the high-level architecture, component topology, and critical user flows of the Smart IoT Watering System. The system comprises a physical ESP32 device, a Supabase Backend-as-a-Service (BaaS) layer, and a Progressive Web App (PWA) built with SvelteKit.

---

## 1. High-Level Component Topology

The system uses a pure serverless/BaaS architecture. Supabase acts as the central source of truth, routing both historical telemetry data via PostgreSQL and real-time commands via its Websockets/Realtime engine. This eliminates the need for an intermediate API layer or a dedicated MQTT broker, significantly reducing moving parts and hosting costs.

```mermaid
graph TD
    subgraph Frontend ["SvelteKit PWA (User)"]
        UI[Dashboard UI]
        AuthClient[Supabase Auth Client]
        RestQuery[REST Client]
        RTChannel[Realtime Channel]
    end

    subgraph Backend ["Supabase (BaaS)"]
        AuthDB[(Auth)]
        DB[(PostgreSQL)]
        Edge[Edge Functions]
        Realtime[Realtime / Websockets]
    end

    subgraph Hardware ["ESP32 Device"]
        Sensors[Soil & Temp Sensors]
        Relay[Water Pump Relay]
        Firmware[C++ / PlatformIO]
    end

    %% Frontend connections
    UI --> AuthClient
    UI --> RestQuery
    UI -.->|Live Updates| RTChannel
    
    AuthClient --> AuthDB
    RestQuery -->|Telemetry History, Commands| DB
    RTChannel <-.->|Postgres Changes| Realtime

    %% Hardware connections
    Firmware -->|POST Telemetry| RestQuery
    Firmware -.->|Subscribe| Realtime
    Firmware -->|1. Setup Token| Edge
    
    %% Internal Backend
    DB --- Realtime
    AuthDB --- Edge
    Edge --- DB
```

---

## 2. Device Provisioning Flow (Captive Portal)

To securely link a headless ESP32 device to an authenticated user's account without hardcoding credentials or requiring a native mobile app (Bluetooth/BLE), the system implements a **Captive Portal** token exchange mechanism.

```mermaid
sequenceDiagram
    autonumber
    actor User
    participant WebApp as Web App (SvelteKit)
    participant Edge as Supabase Edge Function
    participant DB as Supabase DB
    participant ESP32 as ESP32 (SmartPlanter-Setup)

    ESP32->>ESP32: Boots, no ID found
    ESP32->>ESP32: Generates random 6-digit PIN
    Note over ESP32: Displays PIN and MAC on OLED

    User->>WebApp: Clicks "Add Device"
    User->>WebApp: Enters MAC and PIN from OLED
    
    WebApp->>DB: INSERT into `device_pairing_sessions`
    DB-->>WebApp: Returns 201

    ESP32->>ESP32: Listens via Realtime for its MAC
    DB-.->>ESP32: Notifies: "PIN 123456 inserted for MAC AA:BB..."
    
    ESP32->>ESP32: Validates PIN locally (Matched!)
    
    ESP32->>Edge: POST /functions/v1/claim-device { mac, pin }
    Edge->>DB: Verify PIN in `device_pairing_sessions`
    Edge->>DB: INSERT into `devices` (binds to user)
    DB-->>Edge: Returns `device_id`
    
    Edge-->>ESP32: 200 OK + `device_id`
    ESP32->>ESP32: Saves `device_id` to NVS and Restarts
    
    Note over DB, WebApp: Realtime detects new device
    DB-.->>WebApp: Push Notification
    WebApp-->>User: Success animation!
```

---

## 3. Telemetry & Hardware Control Loop

During normal operation, the ESP32 operates autonomously. It pushes sensor readings periodically and polls/listens for manual override commands ("Water Plant" button) initialized by the user in the Web App.

```mermaid
sequenceDiagram
    actor User
    participant App as SvelteKit UI
    participant DB as Postgres (Supabase)
    participant RT as Realtime (Supabase)
    participant ESP32 as ESP32 Firmware
    participant Pump as Physical Pump

    %% Telemetry Flow
    rect rgb(23, 36, 27)
        Note right of ESP32: Standard Telemetry Loop (e.g., Every 30s)
        Note right of ESP32: OR Smart Delta Trigger (Moisture > 5.0)
        ESP32->>DB: POST /rest/v1/telemetry (moisture, temp)
        DB-->>RT: Trigger Insert Event
        RT-.->>App: Update Dashboard Chart Immediately
    end

    %% Command Flow
    rect rgb(47, 92, 67)
        Note left of User: Manual Override Loop
        User->>App: Press & Hold "Water Plant"
        App->>DB: INSERT command `pump_on` (Status: pending)
        DB-->>RT: Trigger Insert Event
        RT-.->>ESP32: Receives `pump_on` command
        ESP32->>Pump: GPIO HIGH (Start Pump)
        ESP32->>DB: UPDATE command set Status: `delivered`
        
        User->>App: Releases "Water Plant"
        App->>DB: INSERT command `pump_off` (Status: pending)
        DB-->>RT: Trigger Insert Event
        RT-.->>ESP32: Receives `pump_off` command
        ESP32->>Pump: GPIO LOW (Stop Pump)
        ESP32->>DB: UPDATE command set Status: `completed`
    end

    %% Automatic Fail-safe & Auto-Watering
    rect rgb(35, 30, 20)
        Note right of ESP32: Hardware Autonomy
        ESP32->>ESP32: If moisture < `auto_water_threshold` -> Start Pump
        ESP32->>ESP32: If running > `pump_duration_seconds` -> Stop Pump
        ESP32->>DB: POST new System Event (Failsafe triggered)
    end
    
    %% Configuration Sync
    rect rgb(30, 45, 75)
        Note left of User: Realtime Configuration
        User->>App: Adjusts Threshold / Duration Slider
        App->>DB: UPDATE `devices` table
        DB-->>RT: Trigger Update Event
        RT-.->>ESP32: Receives updated settings instantly
        ESP32->>ESP32: Applies new settings (No polling)
    end
```

---

## 4. Key Architectural Decisions

1. **SvelteKit Serverless Deployment**: Deploys natively to Vercel. `+page.server.ts` resolves the bulky history queries prior to render, improving load times. Live telemetry binds client-side via `onMount` WebSockets.
2. **Compound B-Tree Indexing**: The `telemetry` table utilizes a structured index `(device_id, recorded_at desc)`. As the IoT database swells to millions of rows, the UI's `GET most recent 48 rows for device` query takes sub-millisecond I/O time.
3. **Data Type Memory Saving**: Sensor metrics utilize `real` float columns over `numeric`, cutting overall table disk storage by 50%.
4. **State Machine over Booleans**: Device commands utilize a Postgres ENUM (`pending`, `delivered`, `completed`, `failed`). This resolves the common IoT edge case where an app assumes a device received a command, but the device actually dropped offline during transit. The UI will visually reflect the pipeline state.
