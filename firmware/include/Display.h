#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// --- SPI Pins ---
#define OLED_MOSI 19
#define OLED_CLK 18
#define OLED_DC 22
#define OLED_CS 23
#define OLED_RST 21

class Display {
public:
  static void setup();
  static void clear();
  static void update();

  static void showProvisioningStatus(const char *ssid, const char *ip);
  static void showWiFiConnecting(const char *ssid);
  static void showWiFiConnected(const char *ip, const char *mdns);
  static void showTelemetry(float moisture, float temp, float hum, const char *ip = nullptr);
  static void showStatus(const char *status, bool isError = false);
  static void showRegistrationAlert(const char *token = nullptr);
  static void showPairingPIN(const char *mac, const char *pin);

private:
  static Adafruit_SSD1306 _display;
};

#endif
