#include "Display.h"
#include "Config.h"
#include <WiFi.h>

Adafruit_SSD1306 Display::_display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI,
                                   OLED_CLK, OLED_DC, OLED_RST, OLED_CS);

void Display::setup() {
  Serial.println("[Display] Initializing SPI OLED...");

  if (!_display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("[Display] SSD1306 allocation failed"));
  }

  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setTextColor(WHITE);
  _display.setCursor(0, 0);
  _display.println("Smart Planter v1.1");
  _display.println("Quality Crafted UI");
  _display.display();
}

void Display::clear() { _display.clearDisplay(); }

void Display::update() {
  // Can be used for background UI refreshes
}

void Display::showProvisioningStatus(const char *ssid, const char *ip) {
  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.println("---- SETUP MODE ----");
  _display.println("");
  _display.println("1. Connect WiFi to:");
  _display.setTextSize(1);
  _display.setTextColor(BLACK, WHITE); // Invert
  _display.println(ssid);
  _display.setTextColor(WHITE);
  _display.println("");
  _display.print("2. Open: ");
  _display.println(ip);
  _display.println("3. Enter your Token.");
  _display.display();
}

void Display::showWiFiConnecting(const char *ssid) {
  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.println("--- CONNECTING ---");
  _display.drawFastHLine(0, 10, 128, WHITE);
  _display.setCursor(0, 30);
  _display.println("Joining:");
  _display.println(ssid);
  _display.display();
}

void Display::showWiFiConnected(const char *ip, const char *mdns) {
  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.println("--- CONNECTED ---");
  _display.drawFastHLine(0, 10, 128, WHITE);
  _display.setCursor(0, 20);
  _display.print("IP: ");
  _display.println(ip);
  _display.println("");
  _display.println("Or visit:");
  _display.setTextColor(BLACK, WHITE);
  _display.print(mdns);
  _display.println(".local");
  _display.setTextColor(WHITE);
  _display.display();
}

void Display::showTelemetry(float moisture, float temp, float hum,
                            const char *ip) {
  _display.clearDisplay();

  // --- Status Bar (Top) ---
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  if (ip != nullptr) {
    _display.print(ip);
  } else {
    _display.print("WiFi: OK  Cloud: ON");
  }
  _display.drawFastHLine(0, 10, 128, WHITE);

  // --- Main Content ---
  // Large Moisture Reading
  _display.setCursor(0, 20);
  _display.setTextSize(4);
  _display.print((int)moisture);
  _display.setTextSize(1);
  _display.print("%");

  _display.setCursor(75, 25);
  _display.print("Moist");

  // Bottom Stats
  _display.drawFastHLine(0, 52, 128, WHITE);
  _display.setCursor(0, 56);
  _display.print((int)temp);
  _display.print("C  ");
  _display.print((int)hum);
  _display.print("% HUMID");

  _display.display();
}

void Display::showStatus(const char *status, bool isError) {
  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.println(isError ? "!! SYSTEM ERROR !!" : "--- SYSTEM UPDATE ---");
  _display.drawFastHLine(0, 10, 128, WHITE);

  _display.setCursor(0, 30);
  _display.setTextSize(2);
  _display.println(status);
  _display.display();
}

void Display::showRegistrationAlert(const char *token) {
  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.println("PAIRING REQUIRED");
  _display.drawFastHLine(0, 10, 128, WHITE);

  _display.setCursor(0, 20);
  _display.println("This device is not linked.");
  _display.println("Use the app to pair.");

  _display.setCursor(0, 48);
  _display.setTextColor(BLACK, WHITE);
  _display.print("Waiting for PIN...");
  _display.setTextColor(WHITE);
  _display.display();
}

void Display::showPairingPIN(const char *mac, const char *pin) {
  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.println("READY TO PAIR");
  _display.drawFastHLine(0, 10, 128, WHITE);

  _display.setCursor(0, 20);
  _display.setTextSize(1);
  _display.print("MAC: ");
  _display.println(mac);

  _display.setCursor(0, 35);
  _display.setTextSize(1);
  _display.println("Enter PIN in App:");

  _display.setCursor(10, 48);
  _display.setTextSize(2);
  _display.setTextColor(BLACK, WHITE);
  _display.print(" ");
  _display.print(pin);
  _display.print(" ");
  _display.setTextColor(WHITE);

  _display.display();
}
