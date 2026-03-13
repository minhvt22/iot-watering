#include "SupabaseClient.h"
#include "Provisioning.h"
#include "Hardware.h"

const char* SupabaseClient::_url = nullptr;
const char* SupabaseClient::_anonKey = nullptr;
WebSocketsClient SupabaseClient::_webSocket;

void SupabaseClient::setup(const char* url, const char* anonKey) {
    _url = url;
    _anonKey = anonKey;

    // WebSocket configuration for Supabase Realtime
    String host = String(url);
    host.replace("https://", "");
    host.replace("/", "");
    
    String wsUrl = "/realtime/v1/websocket?apikey=" + String(anonKey) + "&vsn=1.0.0";
    
    _webSocket.begin(host, 443, wsUrl);
    _webSocket.onEvent(onWebSocketEvent);
    _webSocket.setReconnectInterval(5000);
    _webSocket.enableHeartbeat(30000, 3000, 2);
}

void SupabaseClient::update() {
    _webSocket.loop();
}

bool SupabaseClient::registerDevice(const String& claimToken) {
    if (claimToken.length() == 0) return false;

    HTTPClient http;
    String endpoint = String(_url) + "/functions/v1/register-device";
    
    http.begin(endpoint);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + String(_anonKey));

    JsonDocument doc;
    doc["claim_token"] = claimToken;
    String payload;
    serializeJson(doc, payload);

    int code = http.POST(payload);
    if (code == 200) {
        String response = http.getString();
        JsonDocument resDoc;
        deserializeJson(resDoc, response);
        String deviceId = resDoc["device_id"];
        if (deviceId.length() > 0) {
            Provisioning::setDeviceId(deviceId);
            return true;
        }
    } else {
        Serial.printf("[Supabase] Registration failed (%d): %s\n", code, http.getString().c_str());
    }
    return false;
}

bool SupabaseClient::sendTelemetry(float moisture, float temperature) {
    if (!Provisioning::isConfigured()) return false;

    HTTPClient http;
    String endpoint = String(_url) + "/rest/v1/telemetry";
    
    http.begin(endpoint);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", _anonKey);
    http.addHeader("Authorization", "Bearer " + String(_anonKey));
    http.addHeader("Prefer", "return=minimal");

    JsonDocument doc;
    doc["device_id"] = Provisioning::getDeviceId();
    doc["moisture"] = moisture;
    doc["temperature"] = temperature;

    String payload;
    serializeJson(doc, payload);

    int code = http.POST(payload);
    return code == 201;
}

void SupabaseClient::syncSettings() {
    if (!Provisioning::isConfigured()) return;

    HTTPClient http;
    String endpoint = String(_url) + "/rest/v1/devices?id=eq." + Provisioning::getDeviceId() + "&select=auto_water_threshold,pump_duration_seconds";
    
    http.begin(endpoint);
    http.addHeader("apikey", _anonKey);
    http.addHeader("Authorization", "Bearer " + String(_anonKey));

    int code = http.GET();
    if (code == 200) {
        String response = http.getString();
        JsonDocument doc;
        deserializeJson(doc, response);
        if (doc.is<JsonArray>() && doc.size() > 0) {
            int threshold = doc[0]["auto_water_threshold"];
            int duration = doc[0]["pump_duration_seconds"];
            Hardware::setPumpDurationLimit(duration);
            Serial.printf("[Supabase] Settings synced: Threshold=%d, Duration=%d\n", threshold, duration);
        }
    }
}

void SupabaseClient::updateCommandStatus(long commandId, const char* status) {
    HTTPClient http;
    String endpoint = String(_url) + "/rest/v1/commands?id=eq." + String(commandId);
    
    http.begin(endpoint);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", _anonKey);
    http.addHeader("Authorization", "Bearer " + String(_anonKey));

    JsonDocument doc;
    doc["status"] = status;
    if (strcmp(status, "completed") == 0) {
        // doc["completed_at"] = "now()"; // Usually handled by DB default but good to be explicit if needed
    }

    String payload;
    serializeJson(doc, payload);

    http.sendRequest("PATCH", payload);
}

void SupabaseClient::onWebSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED:
            Serial.println("[WebSocket] Connected to Supabase");
            // Join the real-time channel for commands
            // Format: {"topic":"realtime:public:commands:device_id=eq.UUID","event":"phx_join","payload":{},"ref":"1"}
            if (Provisioning::isConfigured()) {
                String topic = "realtime:public:commands:device_id=eq." + Provisioning::getDeviceId();
                String joinMsg = "{\"topic\":\"" + topic + "\",\"event\":\"phx_join\",\"payload\":{},\"ref\":\"1\"}";
                _webSocket.sendTXT(joinMsg);
            }
            break;
            
        case WStype_TEXT: {
            JsonDocument doc;
            deserializeJson(doc, payload);
            const char* event = doc["event"];
            if (event && (strcmp(event, "INSERT") == 0 || strcmp(event, "UPDATE") == 0)) {
                handleCommand(doc);
            }
            break;
        }
        
        case WStype_DISCONNECTED:
            Serial.println("[WebSocket] Disconnected");
            break;
            
        default:
            break;
    }
}

void SupabaseClient::handleCommand(JsonDocument& doc) {
    JsonObject record = doc["payload"]["new"];
    if (record.isNull()) return;

    long id = record["id"];
    const char* cmd = record["command"];
    const char* status = record["status"];

    if (status && strcmp(status, "pending") == 0) {
        Serial.printf("[Supabase] Received Command: %s (ID: %ld)\n", cmd, id);
        
        // Acknowledge receipt
        updateCommandStatus(id, "delivered");

        if (strcmp(cmd, "pump_on") == 0) {
            Hardware::setPump(true);
            updateCommandStatus(id, "completed");
        } else if (strcmp(cmd, "pump_off") == 0) {
            Hardware::setPump(false);
            updateCommandStatus(id, "completed");
        }
    }
}
