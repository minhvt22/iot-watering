#ifndef SUPABASE_CLIENT_H
#define SUPABASE_CLIENT_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

class SupabaseClient {
public:
    static void setup(const char* url, const char* anonKey);
    static void update();

    static bool registerDevice(const String& claimToken);
    static bool sendTelemetry(float moisture, float temperature);
    static void syncSettings();
    static void updateCommandStatus(long commandId, const char* status);

private:
    static const char* _url;
    static const char* _anonKey;
    static WebSocketsClient _webSocket;
    
    static void onWebSocketEvent(WStype_t type, uint8_t* payload, size_t length);
    static void handleCommand(JsonDocument& doc);
};

#endif
