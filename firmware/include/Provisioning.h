#ifndef PROVISIONING_H
#define PROVISIONING_H

#include <Arduino.h>
#include <Preferences.h>
#include <WiFiManager.h>

class Provisioning {
public:
  typedef void (*ProvisioningCallback)();

  static void setup();
  static void setAPCallback(ProvisioningCallback cb) { _apCallback = cb; }
  static bool isConfigured();

  static bool isInAPMode();
  static String getIP();
  static String getSSID();
  static void reset();

  static String getDeviceId();

  static void setDeviceId(String id);

private:
  static String _deviceId;
  static Preferences _prefs;
  static ProvisioningCallback _apCallback;
};

#endif
