#include "Provisioning.h"
#include "Config.h"
#include <ESPmDNS.h>
#include <WiFi.h>

String Provisioning::_deviceId = "";
Preferences Provisioning::_prefs;
Provisioning::ProvisioningCallback Provisioning::_apCallback = nullptr;

void Provisioning::setup() {
  _prefs.begin(PREFS_NAMESPACE, false);
  _deviceId = _prefs.getString("device_id", "");

  WiFiManager wm;

  // Set callback for when AP mode starts
  wm.setAPCallback([](WiFiManager *myWiFiManager) {
    if (_apCallback)
      _apCallback();
  });

  if (_deviceId.length() == 0 && WiFi.status() != WL_CONNECTED) {
    Serial.println("[Provisioning] Connecting to stored WiFi...");
  }

  // If not configured, this starts an Access Point
  if (!wm.autoConnect(CAPTIVE_PORTAL_SSID)) {
    Serial.println("[Provisioning] Failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
  }

  // Setup mDNS
  if (MDNS.begin(MDNS_NAME)) {
    Serial.printf("[Provisioning] mDNS responder started: %s.local\n",
                  MDNS_NAME);
  }

  Serial.println("[Provisioning] WiFi Connected");
}

bool Provisioning::isConfigured() { return _deviceId.length() > 0; }

bool Provisioning::isInAPMode() { return (WiFi.getMode() & WIFI_MODE_AP) != 0; }

String Provisioning::getIP() {
  if (isInAPMode())
    return WiFi.softAPIP().toString();
  return WiFi.localIP().toString();
}

String Provisioning::getSSID() {
  if (isInAPMode())
    return CAPTIVE_PORTAL_SSID;
  return WiFi.SSID();
}

void Provisioning::reset() {
  _prefs.clear();
  WiFiManager wm;
  wm.resetSettings();
  ESP.restart();
}

String Provisioning::getDeviceId() { return _deviceId; }

void Provisioning::setDeviceId(String id) {
  _deviceId = id;
  _prefs.putString("device_id", id);
  Serial.printf("[Provisioning] Device ID stored: %s\n", id.c_str());
}
