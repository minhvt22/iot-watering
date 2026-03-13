#ifndef PROVISIONING_H
#define PROVISIONING_H

#include <Arduino.h>
#include <WiFiManager.h>
#include <Preferences.h>

class Provisioning {
public:
    static void setup();
    static bool isConfigured();
    static void reset();
    
    static String getDeviceId();
    static String getOwnerId();
    static String getClaimToken();

    static void setDeviceId(String id);

private:
    static String _deviceId;
    static String _claimToken;
    static Preferences _prefs;
};

#endif
