# User Acceptance Testing (UAT) Guide: Web Dashboard v1.1

This document outlines the testing procedures to verify the web application's functionality, UI responsiveness, and integration with the IoT hardware.

## 1. Environment Requirements

- **Browser**: Chrome (Latest), Safari (iOS), or Edge.
- **Auth**: A valid Supabase Auth account (Google or Email).

---

## 2. Test Scenarios

### W-01: Authentication & Onboarding

**Objective**: Verify secure access and initial state.

| Step | Action                       | Expected Result                                          | Pass/Fail |
| :--- | :--------------------------- | :------------------------------------------------------- | :-------: |
| 1.1  | Visit the login page.        | M3 compliant UI with "Sign In" options.                  |           |
| 1.2  | Log in with a new account.   | Redirects to Dashboard. Shows "No devices linked" FAB.   |           |
| 1.3  | Check Mobile Responsiveness. | UI elements stack correctly; Sidebar becomes bottom nav. |           |

### W-02: Device Pairing Flow (PIN Alignment)

**Objective**: Verify the pairing handshake from the user's perspective.

| Step | Action                                      | Expected Result                                  | Pass/Fail |
| :--- | :------------------------------------------ | :----------------------------------------------- | :-------: |
| 2.1  | Click the "Add Device" (Plus) button.       | Clean dialog pops up asking for MAC and PIN.     |           |
| 2.2  | Enter a random 12-char MAC and 6-digit PIN. | "Pair Device" button becomes active.             |           |
| 2.3  | Submit pairing.                             | Dialog transitions to "Connecting..." state.     |           |
| 2.4  | Simulating Hardware: Handshake matches.     | Dialog shows "Device Paired!" with Success Icon. |           |

### W-03: Realtime Dashboard Monitoring

**Objective**: Verify live telemetry visualization.

| Step | Action                                  | Expected Result                                  | Pass/Fail |
| :--- | :-------------------------------------- | :----------------------------------------------- | :-------: |
| 3.1  | Select a paired device.                 | Moisture, Temp, and Humidity metrics appear.     |           |
| 3.2  | Wait for automatic update (30s).        | Chart data points update via Realtime every 30s. |           |
| 3.3  | Trigger significant change on hardware. | Chart should update immediately (Delta Trigger). |           |

### W-04: Remote Hardware Control

**Objective**: Verify the bidirectional command flow.

| Step | Action                      | Expected Result                                        | Pass/Fail |
| :--- | :-------------------------- | :----------------------------------------------------- | :-------: |
| 4.1  | Click "Water Plant" button. | Button transitions to "Active" state with ripple.      |           |
| 4.2  | Observe "Pump Indicator".   | Realtime indicator glows while pump is active.         |           |
| 4.3  | Check Command History logs. | A new `pump_on` entry appears with "completed" status. |           |

### W-05: Advanced Configuration Sync

**Objective**: Verify that UI sliders properly control device settings in real-time.

| Step | Action | Expected Result | Pass/Fail |
| :--- | :--- | :--- | :---: |
| 5.1 | Expand "Advanced Settings" accordion. | Accordion opens smoothly revealing Range sliders. | |
| 5.2 | Drag the Auto-Water Threshold slider. | The value updates dynamically next to the slider handle. | |
| 5.3 | Release slider. | UI debounces save (1s) and updates the database without full page reload. | |
| 5.4 | Hardware check. | Hardware physically reacts (or logs) applying the setting immediately via Websocket. | |

---

## 3. Product Excellence Criteria (UX)

- [ ] No layout shifts during Realtime data ingestion.
- [ ] Smooth transitions between Dashboard and Device Settings.
- [ ] Dark Mode support: UI switches based on OS preference.
- [ ] Error Handling: Offline devices show a clear "Disconnected" warning.

**Tester Signature**: **\*\*\*\***\_\_\_\_**\*\*\*\*** **Date**: \***\*\_\_\*\***
