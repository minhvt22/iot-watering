#ifndef SUPABASE_CLIENT_H
#define SUPABASE_CLIENT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WebSocketsClient.h>

/**
 * @brief SupabaseClient handles WebSocket (Realtime v2) and REST communications
 * with Supabase. It manages device pairing, telemetry, and command execution.
 */
class SupabaseClient {
public:
  enum class Event {
    CONNECTED,
    DISCONNECTED,
    ERROR,
    COMMAND_RECEIVED,
    PAIRING_START,
    PAIRING_DONE,
    PAIRING_TIMEOUT,
    SUBSCRIBED,
    SETTINGS_SYNCED
  };

  typedef void (*Callback)(Event event, const char *data);

  static void setup(const char *url, const char *anonKey);
  static void update();
  static void setCallback(Callback cb) { _callback = cb; }

  static bool registerDevice(const String &pin);
  static bool sendTelemetry(float moisture, float temperature,
                            bool force = false);
  static void syncSettings();
  static void updateCommandStatus(long commandId, const char *status);
  static void startPairingFlow();
  static bool isPairing() { return _isPairing; }

private:
  static void onWebSocketEvent(WStype_t type, uint8_t *payload, size_t length);
  static void handleConnected();
  static void handleMessage(JsonDocument &doc);
  static void handleRealtimeEvent(JsonDocument &doc);
  static void handlePairingStep(JsonDocument &doc);
  static void sendHeartbeat();

  static const char *_url;
  static const char *_anonKey;
  static WebSocketsClient _webSocket;
  static String _pairingPIN;
  static bool _isPairing;
  static unsigned long _pairingStartTime;
  static unsigned long _lastHeartbeat;
  static unsigned int _msgRef;
  static String _headerStore;
  static Callback _callback;

  // Telemetry state for smart intervals
  static unsigned long _lastTelemetryMillis;
  static float _lastMoisture;
  static float _lastTemperature;
};

#endif
