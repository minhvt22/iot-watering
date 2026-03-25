# User Acceptance Testing (UAT) Guide: Smart Planter v1.1

This document outlines the testing procedures to verify that the firmware meets the functional and experience requirements for the end-user.

## 1. Hardware Environment
- **Device**: ESP32 DevKit V1
- **Visual**: SSD1306 OLED (128x64) via SPI
- **Audio**: Passive/Active Buzzer (Pin 25)
- **Sensing**: DHT11 (Temp/Hum), Soil Moisture Sensor (Analog)
- **Actuator**: 5V Relay / Water Pump (Pin 26)

---

## 2. Test Scenarios

### T-01: Out-of-the-Box Experience (OOBE)
**Objective**: Verify the first-time setup and WiFi connection.

| Step | Action | Expected Result | Pass/Fail |
| :--- | :--- | :--- | :---: |
| 1.1 | Power on the device. | **OLED**: Shows "Smart Planter" then "SETUP MODE".<br>**Audio**: Plays a rising 4-note boot melody. | |
| 1.2 | Search for WiFi on your phone. | SSID `SmartPlanter-Setup` should be visible. | |
| 1.3 | Connect to `SmartPlanter-Setup`. | Captive portal should pop up (or visit 192.168.4.1). | |
| 1.4 | Submit WiFi credentials and click "Connect". | **OLED**: Device connects to WiFi, then shows "READY TO PAIR" with a MAC address and 6-digit PIN. | |

### T-02: Cloud Pairing & Registration (Modern Handshake)
**Objective**: Verify the device successfully links to the user account via PIN.

| Step | Action | Expected Result | Pass/Fail |
| :--- | :--- | :--- | :---: |
| 2.1 | Open the web app and click "Add Device". | Pairing dialog appears. | |
| 2.2 | Enter the MAC and PIN shown on the device screen. | **Web**: Shows "Connecting..." state. | |
| 2.3 | Wait for the device handshake. | **OLED**: Shows "Pairing Success" then restarts.<br>**Web**: Plays success animation and opens dashboard.<br>**Audio**: Plays cheerful "Success" chirp. | |
| 2.4 | Check dashboard telemetry. | Live moisture data should appear immediately. | |

### T-03: Live Monitoring & Display
**Objective**: Verify the "Product-Grade" UI and data accuracy.

| Step | Action | Expected Result | Pass/Fail |
| :--- | :--- | :--- | :---: |
| 3.1 | Observe the main OLED screen. | **Status Bar**: Shows "WiFi: OK" and "Cloud: ON".<br>**Center**: Large moisture % reading. | |
| 3.2 | Dip moisture sensor in water. | Reading on OLED should increase immediately. | |
| 3.3 | Blow on DHT11 sensor. | Temp/Humidity values at the bottom should update on OLED immediately. | |
| 3.4 | Observe Serial Monitor (Smart Telemetry). | New rows should appear in Supabase every 30s OR immediately if moisture changes > 5%. (Min 5s gap). | |

### T-04: Remote Command Execution
**Objective**: Verify the low-latency response to web commands.

| Step | Action | Expected Result | Pass/Fail |
| :--- | :--- | :--- | :---: |
| 4.1 | Send `pump_on` from the Web App. | **Pump**: Relay clicks ON.<br>**Audio**: Plays "Watering Start" tone.<br>**Web**: Command status moves to `completed`. | |
| 4.2 | Send `pump_off` from the Web App. | **Pump**: Relay clicks OFF.<br>**Audio**: Plays "Watering Stop" tone. | |

### T-05: Safety Failsafes
**Objective**: Verify the hardware protection logic.

| Step | Action | Expected Result | Pass/Fail |
| :--- | :--- | :--- | :---: |
| 5.1 | Send `pump_on` and leave it running. | After 15s (default limit), the pump must auto-shutdown. | |
| 5.2 | Observe OLED during failsafe. | **Audio**: Plays an aggressive error alarm sequence. | |

### T-06: Advanced Settings & Auto-Watering
**Objective**: Verify real-time configuration sync and autonomous watering logic.

| Step | Action | Expected Result | Pass/Fail |
| :--- | :--- | :--- | :---: |
| 6.1 | Adjust Auto-Water Threshold in Web Dashboard. | **Serial**: Logs "Realtime Settings Update Received: [X]|[Y]". | |
| 6.2 | Ensure soil moisture is below the new threshold. | **Pump**: Triggers automatically.<br>**Serial**: Logs "Auto-Watering Triggered!". | |
| 6.3 | Adjust Maximum Pump Duration to 5s. | Future auto-water or manual overrides automatically halt after exactly 5 seconds. | |

---

## 3. Approval Criteria
- [ ] Device boots and connects in < 15 seconds.
- [ ] Command latency (Web to Pump) is < 2 seconds.
- [ ] No "Zombie Mode" when cloud connection fails (Retries gracefully).
- [ ] All magic numbers (pins, intervals) are centralized in Config.h.

**Tester Signature**: ____________________  **Date**: __________
