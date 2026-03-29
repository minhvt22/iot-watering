#include "SupabaseClient.h"
#include "Config.h"
#include "Provisioning.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#define SUPABASE_LOG(fmt, ...)                                                 \
  Serial.printf("[Supabase] " fmt "\n", ##__VA_ARGS__)

const char *SupabaseClient::_url = nullptr;
const char *SupabaseClient::_anonKey = nullptr;
WebSocketsClient SupabaseClient::_webSocket;
String SupabaseClient::_pairingPIN = "";
bool SupabaseClient::_isPairing = false;
unsigned long SupabaseClient::_pairingStartTime = 0;
unsigned long SupabaseClient::_lastHeartbeat = 0;
unsigned int SupabaseClient::_msgRef = 0;
String SupabaseClient::_headerStore = "";
SupabaseClient::Callback SupabaseClient::_callback = nullptr;

unsigned long SupabaseClient::_lastTelemetryMillis = 0;
float SupabaseClient::_lastMoisture = -100.0f;
float SupabaseClient::_lastTemperature = -100.0f;

void SupabaseClient::setup(const char *url, const char *anonKey) {
  _url = url;
  _anonKey = anonKey;

  String host = String(url);
  host.replace("https://", "");
  host.replace("/", "");

  String wsUrl =
      "/realtime/v1/websocket?apikey=" + String(anonKey) + "&vsn=2.0.0";

  SUPABASE_LOG("Host: %s", host.c_str());
  SUPABASE_LOG("Key (prefix): %.10s...", anonKey);

  _webSocket.beginSslWithCA(host.c_str(), 443, wsUrl.c_str(), NULL, "");
  _webSocket.onEvent(onWebSocketEvent);
  _webSocket.setReconnectInterval(RECONNECT_INTERVAL_MS);

  _headerStore = "User-Agent: ESP32-IOT-Planter\r\napikey: " + String(anonKey);
  _webSocket.setExtraHeaders(_headerStore.c_str());
}

void SupabaseClient::update() {
  _webSocket.loop();
  sendHeartbeat();

  if (!_isPairing)
    return;

  unsigned long now = millis();
  if (now - _pairingStartTime >= PAIRING_TIMEOUT_MS) {
    SUPABASE_LOG("Pairing Timed Out (%lu ms)", now - _pairingStartTime);
    _isPairing = false;
    _pairingPIN = "";
    if (_callback)
      _callback(SupabaseClient::Event::PAIRING_TIMEOUT, nullptr);
  }
}

void SupabaseClient::onWebSocketEvent(WStype_t type, uint8_t *payload,
                                      size_t length) {
  switch (type) {
  case WStype_CONNECTED:
    SUPABASE_LOG("Connected!");
    handleConnected();
    if (_callback)
      _callback(SupabaseClient::Event::CONNECTED, nullptr);
    break;

  case WStype_DISCONNECTED: {
    String reason = (payload && length > 0)
                        ? String((char *)payload).substring(0, length)
                        : "N/A";
    SUPABASE_LOG("Disconnected. Reason: %s", reason.c_str());
    if (_callback)
      _callback(SupabaseClient::Event::DISCONNECTED, nullptr);
    break;
  }

  case WStype_ERROR: {
    String err = (payload && length > 0)
                     ? String((char *)payload).substring(0, length)
                     : "Unknown";
    SUPABASE_LOG("Socket Error: %s", err.c_str());
    if (_callback)
      _callback(SupabaseClient::Event::ERROR, err.c_str());
    break;
  }

  case WStype_TEXT: {
    JsonDocument doc;
    if (!deserializeJson(doc, payload, length)) {
      handleMessage(doc);
    }
    break;
  }
  default:
    break;
  }
}

void SupabaseClient::handleMessage(JsonDocument &doc) {
  if (!doc.is<JsonArray>() || doc.size() < 5)
    return;

  const char *topic = doc[2];
  const char *event = doc[3];
  JsonObject payload = doc[4];

  if (strcmp(event, "phx_reply") == 0) {
    if (payload["status"] && strcmp(payload["status"], "ok") == 0) {
      SUPABASE_LOG("Subscribed to %s", topic);
    }
    return;
  }

  if (strcmp(event, "postgres_changes") != 0)
    return;

  if (strstr(topic, "pairing")) {
    handlePairingStep(doc);
  } else if (strstr(topic, "device_events")) {
    handleRealtimeEvent(doc);
  }
}

void SupabaseClient::handlePairingStep(JsonDocument &doc) {
  JsonObject payload = doc[4];
  JsonObject record = payload["data"]["record"];
  if (record.isNull())
    record = payload["record"];
  if (record.isNull())
    record = payload["data"]["new"];
  if (record.isNull())
    record = payload["new"];

  if (record.isNull())
    return;

  const char *pin = record["pin"];
  if (pin && strcmp(pin, _pairingPIN.c_str()) == 0) {
    SUPABASE_LOG("PIN Match! Finalizing registration...");
    if (registerDevice(_pairingPIN)) {
      if (_callback)
        _callback(SupabaseClient::Event::PAIRING_DONE, nullptr);
    }
  }
}

void SupabaseClient::handleRealtimeEvent(JsonDocument &doc) {
  JsonObject payload = doc[4];
  const char *table = payload["data"]["table"];
  if (!table)
    table = payload["table"];

  JsonObject record = payload["data"]["record"];
  if (record.isNull())
    record = payload["record"];
  if (record.isNull())
    record = payload["data"]["new"];
  if (record.isNull())
    record = payload["new"];

  if (record.isNull())
    return;

  // 1. Device Table Settings Update
  if (table && strcmp(table, "devices") == 0) {
    if (record["auto_water_threshold"].is<int>()) {
      int duration = record["pump_duration_seconds"];
      int threshold = record["auto_water_threshold"];
      String settings = String(duration) + "|" + String(threshold);
      SUPABASE_LOG("Realtime Settings Update Received: %s", settings.c_str());
      if (_callback)
        _callback(SupabaseClient::Event::SETTINGS_SYNCED, settings.c_str());
    }
    return;
  }

  // 2. Commands Table INSERT
  long id = record["id"];
  const char *cmd = record["command"];
  const char *status = record["status"];

  // Note: Telemetry commands like 'water_now'
  if (status && strcmp(status, "pending") == 0) {
    SUPABASE_LOG("Command Received: %s", cmd);
    if (_callback)
      _callback(SupabaseClient::Event::COMMAND_RECEIVED, cmd);
    updateCommandStatus(id, "completed");
  }
}

void SupabaseClient::startPairingFlow() {
  _pairingStartTime = millis();
  _isPairing = true;

  randomSeed(micros());
  _pairingPIN = String(random(PIN_MIN_RANGE, PIN_MAX_RANGE));

  String mac = WiFi.macAddress();
  mac.replace(":", "");

  SUPABASE_LOG("Pairing Flow Init. PIN: %s, MAC: %s", _pairingPIN.c_str(),
               mac.c_str());

  if (_callback) {
    String data = mac + "|" + _pairingPIN;
    _callback(SupabaseClient::Event::PAIRING_START, data.c_str());
  }

  if (_webSocket.isConnected()) {
    String ref = String(++_msgRef);
    String topic = "realtime:pairing";
    String joinMsg =
        "[\"" + ref + "\",\"" + ref + "\",\"" + topic +
        "\",\"phx_join\",{\"config\":{\"postgres_changes\":[{\"event\":"
        "\"INSERT\",\"schema\":\"public\",\"table\":\"device_pairing_"
        "sessions\",\"filter\":\"mac_address=eq." +
        mac + "\"}],\"user_token\":\"" + String(_anonKey) + "\"}}]";
    _webSocket.sendTXT(joinMsg);
  }
}

void SupabaseClient::sendHeartbeat() {
  if (!_webSocket.isConnected() || (millis() - _lastHeartbeat < 30000))
    return;

  _lastHeartbeat = millis();
  String ref = String(++_msgRef);
  _webSocket.sendTXT("[null,\"" + ref + "\",\"phoenix\",\"heartbeat\",{}]");
}

void SupabaseClient::updateCommandStatus(long commandId, const char *status) {
  HTTPClient http;
  String endpoint =
      String(_url) + "/rest/v1/commands?id=eq." + String(commandId);
  http.begin(endpoint);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", _anonKey);
  http.addHeader("Authorization", "Bearer " + String(_anonKey));

  JsonDocument doc;
  doc["status"] = status;
  String json;
  serializeJson(doc, json);
  http.sendRequest("PATCH", json);
}

void SupabaseClient::handleConnected() {
  String mac = WiFi.macAddress();
  mac.replace(":", "");

  if (_isPairing) {
    String ref = String(++_msgRef);
    String topic = "realtime:pairing";
    String joinMsg =
        "[\"" + ref + "\",\"" + ref + "\",\"" + topic +
        "\",\"phx_join\",{\"config\":{\"postgres_changes\":[{\"event\":"
        "\"INSERT\",\"schema\":\"public\",\"table\":\"device_pairing_"
        "sessions\",\"filter\":\"mac_address=eq." +
        mac + "\"}]}}]";
    _webSocket.sendTXT(joinMsg);
  }

  if (Provisioning::isConfigured()) {
    String ref = String(++_msgRef);
    String topic = "realtime:device_events";
    String did = Provisioning::getDeviceId();
    String joinMsg = "[\"" + ref + "\",\"" + ref + "\",\"" + topic +
                     "\",\"phx_join\",{\"config\":{\"postgres_changes\":["
                     "{\"event\":\"INSERT\",\"schema\":\"public\",\"table\":"
                     "\"commands\",\"filter\":\"device_id=eq." +
                     did +
                     "\"},"
                     "{\"event\":\"UPDATE\",\"schema\":\"public\",\"table\":"
                     "\"devices\",\"filter\":\"id=eq." +
                     did +
                     "\"}"
                     "],\"user_token\":\"" +
                     String(_anonKey) + "\"}}]";
    _webSocket.sendTXT(joinMsg);
    if (_callback)
      _callback(SupabaseClient::Event::SUBSCRIBED, topic.c_str());
  }
}

bool SupabaseClient::registerDevice(const String &pin) {
  HTTPClient http;
  http.begin(String(_url) + "/functions/v1/claim-device");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(_anonKey));

  JsonDocument doc;
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  doc["mac_address"] = mac;
  doc["pin"] = pin;

  String body;
  serializeJson(doc, body);
  int code = http.POST(body);
  if (code != 200)
    return false;

  JsonDocument res;
  deserializeJson(res, http.getString());
  String deviceId = res["device_id"];
  if (deviceId.length() == 0)
    return false;

  Provisioning::setDeviceId(deviceId);
  _isPairing = false;
  return true;
}

bool SupabaseClient::sendTelemetry(float moisture, float temperature,
                                   bool force) {
  if (!Provisioning::isConfigured())
    return false;

  unsigned long now = millis();
  bool intervalTrigger = (now - _lastTelemetryMillis >= TELEMETRY_INTERVAL_MS);
  bool deltaTrigger = (abs(moisture - _lastMoisture) > 5.0f) ||
                      (abs(temperature - _lastTemperature) > 1.0f);
  bool cooldownPassed = (now - _lastTelemetryMillis >= 5000);

  if (!force && !intervalTrigger && (!deltaTrigger || !cooldownPassed)) {
    return false;
  }

  HTTPClient http;
  http.begin(String(_url) + "/rest/v1/telemetry");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", _anonKey);
  http.addHeader("Authorization", "Bearer " + String(_anonKey));

  JsonDocument doc;
  doc["device_id"] = Provisioning::getDeviceId();
  doc["moisture"] = moisture;
  doc["temperature"] = temperature;

  String body;
  serializeJson(doc, body);
  int code = http.POST(body);

  if (code == 201) {
    _lastTelemetryMillis = now;
    _lastMoisture = moisture;
    _lastTemperature = temperature;
    return true;
  }
  return false;
}

void SupabaseClient::syncSettings() {
  if (!Provisioning::isConfigured())
    return;

  HTTPClient http;
  String url = String(_url) + "/rest/v1/devices?id=eq." +
               Provisioning::getDeviceId() +
               "&select=pump_duration_seconds,auto_water_threshold";
  http.begin(url);
  http.addHeader("apikey", _anonKey);
  http.addHeader("Authorization", "Bearer " + String(_anonKey));

  if (http.GET() == 200) {
    JsonDocument doc;
    deserializeJson(doc, http.getString());
    if (doc.is<JsonArray>() && doc.size() > 0) {
      int duration = doc[0]["pump_duration_seconds"];
      int threshold = doc[0]["auto_water_threshold"];
      String settings = String(duration) + "|" + String(threshold);
      if (_callback)
        _callback(SupabaseClient::Event::SETTINGS_SYNCED, settings.c_str());
    }
  }
}
