#include "Config.h"
#include "Display.h"
#include "Hardware.h"
#include "Provisioning.h"
#include "Sound.h"
#include "SupabaseClient.h"
#include <Arduino.h>
#include <WiFi.h>

enum class AppState {
  PROVISIONING,
  WIFI_CONNECTING,
  WIFI_CONNECTED_SPLASH,
  UNPAIRED,
  ACTIVE
};

AppState currentState;
unsigned long stateStartTime = 0;
unsigned long lastSettingsSync = 0;
unsigned long lastOledRefresh = 0;
wl_status_t lastWifiStatus = WL_IDLE_STATUS;

// --- Event Handlers ---
void onSupabaseEvent(SupabaseClient::Event event, const char *data) {
  switch (event) {
  case SupabaseClient::Event::CONNECTED:
    Display::showStatus("Online");
    break;
  case SupabaseClient::Event::DISCONNECTED:
    Display::showStatus("Offline");
    break;
  case SupabaseClient::Event::ERROR:
    Display::showStatus("Socket Error");
    Sound::play(SoundEvent::ERROR_ALERT);
    break;
  case SupabaseClient::Event::PAIRING_START: {
    String s(data);
    int sep = s.indexOf('|');
    if (sep != -1) {
      Display::showPairingPIN(s.substring(0, sep).c_str(),
                              s.substring(sep + 1).c_str());
      Sound::play(SoundEvent::NOTIFICATION);
    }
    break;
  }
  case SupabaseClient::Event::PAIRING_TIMEOUT:
    Display::showStatus("Pairing Expired");
    Sound::play(SoundEvent::ERROR_ALERT);
    break;
  case SupabaseClient::Event::PAIRING_DONE:
    Display::showStatus("Paired!");
    Sound::play(SoundEvent::NOTIFICATION);
    delay(2000);
    ESP.restart();
    break;
  case SupabaseClient::Event::COMMAND_RECEIVED:
    if (strcmp(data, "pump_on") == 0) {
      Sound::play(SoundEvent::WATERING_START);
      Hardware::setPump(true);
    } else if (strcmp(data, "pump_off") == 0) {
      Hardware::setPump(false);
      Sound::play(SoundEvent::WATERING_STOP);
    }
    break;
  case SupabaseClient::Event::SUBSCRIBED:
    Sound::play(SoundEvent::WIFI_CONNECTED);
    break;
  case SupabaseClient::Event::SETTINGS_SYNCED: {
    if (!data) break;
    String s(data);
    int sep = s.indexOf('|');
    if (sep == -1) break;
    Hardware::setPumpDurationLimit(s.substring(0, sep).toInt());
    Hardware::setAutoWaterThreshold(s.substring(sep + 1).toInt());
    break;
  }
  }
}

void transitionTo(AppState newState) {
  currentState = newState;
  stateStartTime = millis();
  Display::clear();
  Serial.printf("[Main] FSM Transition: %d\n", (int)newState);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Smart IoT Planter: Premium Edition ---");

  Hardware::setup();
  Display::setup();
  Provisioning::setup();
  Sound::play(SoundEvent::BOOT_UP);

  SupabaseClient::setCallback(onSupabaseEvent);
  Provisioning::setAPCallback([]() {
    Display::showProvisioningStatus(CAPTIVE_PORTAL_SSID,
                                    WiFi.softAPIP().toString().c_str());
    Sound::play(SoundEvent::NOTIFICATION);
  });

  SupabaseClient::setup(SUPABASE_URL, SUPABASE_KEY);
  lastWifiStatus = WiFi.status();

  if (Provisioning::isInAPMode())
    transitionTo(AppState::PROVISIONING);
  else if (WiFi.status() != WL_CONNECTED)
    transitionTo(AppState::WIFI_CONNECTING);
  else if (!Provisioning::isConfigured())
    transitionTo(AppState::UNPAIRED);
  else
    transitionTo(AppState::ACTIVE);
}

void updateDisplay() {
  switch (currentState) {
  case AppState::PROVISIONING:
    Display::showProvisioningStatus(Provisioning::getSSID().c_str(),
                                    Provisioning::getIP().c_str());
    break;
  case AppState::WIFI_CONNECTING:
    Display::showWiFiConnecting(Provisioning::getSSID().c_str());
    break;
  case AppState::WIFI_CONNECTED_SPLASH:
    Display::showWiFiConnected(WiFi.localIP().toString().c_str(), MDNS_NAME);
    if (millis() - stateStartTime > 5000) {
      transitionTo(Provisioning::isConfigured() ? AppState::ACTIVE
                                                : AppState::UNPAIRED);
    }
    break;
  case AppState::UNPAIRED:
    if (!SupabaseClient::isPairing())
      SupabaseClient::startPairingFlow();
    else
      Display::showStatus("Ready to Pair...");
    break;
  case AppState::ACTIVE:
    if (millis() - lastOledRefresh >= 2000) {
      lastOledRefresh = millis();
      Display::showTelemetry(
          Hardware::readMoisture(), Hardware::readTemperature(),
          Hardware::readHumidity(), WiFi.localIP().toString().c_str());
    }
    break;
  }
}

void loop() {
  Hardware::update();
  SupabaseClient::update();

  wl_status_t wifiStatus = WiFi.status();
  if (wifiStatus != lastWifiStatus) {
    if (wifiStatus == WL_CONNECTED)
      transitionTo(AppState::WIFI_CONNECTED_SPLASH);
    else if (currentState != AppState::PROVISIONING)
      transitionTo(AppState::WIFI_CONNECTING);
    lastWifiStatus = wifiStatus;
  }

  if (currentState == AppState::UNPAIRED && Provisioning::isConfigured())
    transitionTo(AppState::ACTIVE);

  updateDisplay();

  if (currentState == AppState::ACTIVE) {
    SupabaseClient::sendTelemetry(Hardware::readMoisture(),
                                  Hardware::readTemperature());
    if (millis() - lastSettingsSync >= SETTINGS_SYNC_INTERVAL_MS) {
      lastSettingsSync = millis();
      SupabaseClient::syncSettings();
    }
  }

  delay(10);
}
