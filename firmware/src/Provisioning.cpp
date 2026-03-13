#include "Provisioning.h"

String Provisioning::_deviceId = "";
String Provisioning::_claimToken = "";
Preferences Provisioning::_prefs;

void Provisioning::setup() {
    _prefs.begin("water-sys", false);
    _deviceId = _prefs.getString("device_id", "");
    
    WiFiManager wm;
    
    // Custom parameter for Claim Token
    WiFiManagerParameter custom_claim_token("claim_token", "Supabase Claim Token", "", 10);
    wm.addParameter(&custom_claim_token);

    // If not configured, this starts an Access Point
    if (!wm.autoConnect("SmartPlanter-Setup")) {
        Serial.println("[Provisioning] Failed to connect and hit timeout");
        delay(3000);
        ESP.restart();
    }

    // Capture claim token if it was provided in the portal
    if (wm.getConfigPortalSSID() != "") {
        _claimToken = String(custom_claim_token.getValue());
        Serial.printf("[Provisioning] Captured Claim Token: %s\n", _claimToken.c_str());
    }

    Serial.println("[Provisioning] WiFi Connected");
}

bool Provisioning::isConfigured() {
    return _deviceId.length() > 0;
}

void Provisioning::reset() {
    _prefs.clear();
    WiFiManager wm;
    wm.resetSettings();
    ESP.restart();
}

String Provisioning::getDeviceId() {
    return _deviceId;
}

String Provisioning::getClaimToken() {
    return _claimToken;
}

void Provisioning::setDeviceId(String id) {
    _deviceId = id;
    _prefs.putString("device_id", id);
    Serial.printf("[Provisioning] Device ID stored: %s\n", id.c_str());
}
