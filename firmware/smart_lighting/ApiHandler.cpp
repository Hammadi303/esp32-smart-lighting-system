#include "ApiHandler.h"
#include <ArduinoJson.h>

static constexpr size_t MAX_BODY = 256;

static ApiResponse jsonResponse(int status, const JsonDocument& doc) {
  ApiResponse r; r.status = status;
  serializeJson(doc, r.body);
  return r;
}

static ApiResponse errorResponse(int status, const char* code, const char* msg) {
  JsonDocument d;
  d["error"] = code;
  d["message"] = msg;
  return jsonResponse(status, d);
}

ApiHandler::ApiHandler(DarknessDetector& det, RelayController& relay, WiFiManager& wifi,
                       ConfigStore& store, const SensorData& sensor)
  : _det(det), _relay(relay), _wifi(wifi), _store(store), _sensor(sensor) {}

ApiResponse ApiHandler::handle(const String& method, const String& path,
                               const String& body, uint32_t nowMs) {
  const bool isGet  = (method == "GET");
  const bool isPost = (method == "POST");

  if (path == "/api/status") {
    return isGet ? getStatus(nowMs) : errorResponse(405, "method_not_allowed", "Use GET");
  }
  if (path == "/api/light") {
    return isPost ? postLight(body) : errorResponse(405, "method_not_allowed", "Use POST");
  }
  if (path == "/api/config") {
    if (isGet)  return getConfig();
    if (isPost) return postConfig(body);
    return errorResponse(405, "method_not_allowed", "Use GET or POST");
  }
  return errorResponse(404, "not_found", "Unknown endpoint");
}

ApiResponse ApiHandler::getStatus(uint32_t nowMs) {
  JsonDocument doc;
  doc["api_version"] = 1;
  doc["uptime_s"] = nowMs / 1000;

  JsonObject w = doc["wifi"].to<JsonObject>();
  w["connected"] = _wifi.connected();
  if (_wifi.connected()) { w["ip"] = _wifi.ip(); w["rssi"] = _wifi.rssi(); }
  else                   { w["ip"] = nullptr;    w["rssi"] = nullptr; }

  JsonObject s = doc["sensor"].to<JsonObject>();
  s["adc_raw"] = _sensor.raw;
  s["adc_filtered"] = (int)roundf(_sensor.filtered);
  s["brightness_pct"] = roundf(_sensor.brightnessPct * 10.0f) / 10.0f;

  doc["environment"] = _det.isDark() ? "dark" : "bright";
  doc["fsm_state"] = DarknessDetector::stateName(_det.state());
  doc["dark_event_id"] = _det.eventId();
  if (_det.eventId() > 0) doc["dark_event_age_s"] = (nowMs - _det.eventStartMs()) / 1000;
  else                    doc["dark_event_age_s"] = nullptr;

  doc["relay_state"] = _relay.isOn() ? "on" : "off";
  doc["lora"]["initialized"] = _loraInit;
  return jsonResponse(200, doc);
}



ApiResponse ApiHandler::postLight(const String& body) {
  JsonDocument doc;

  if (body.length() > MAX_BODY || deserializeJson(doc, body)) {
    return errorResponse(400, "invalid_json", "Body must be valid JSON (max 256 bytes)");
  }

  if (!doc["state"].is<const char*>()) {
    return errorResponse(400, "invalid_state", "Field 'state' must be \"on\" or \"off\"");
  }

  const char* state = doc["state"].as<const char*>();

  if (strcmp(state, "on") == 0) {
    _relay.set(true);
    digitalWrite(2, HIGH);

    Serial.println("Relay: ON");
    Serial.println("ESP32 LED: ON");
  }
  else if (strcmp(state, "off") == 0) {
    _relay.set(false);
    digitalWrite(2, LOW);

    Serial.println("Relay: OFF");
    Serial.println("ESP32 LED: OFF");
  }
  else {
    return errorResponse(400, "invalid_state", "Field 'state' must be \"on\" or \"off\"");
  }

  JsonDocument out;
  out["relay_state"] = _relay.isOn() ? "on" : "off";

  return jsonResponse(200, out);
}



ApiResponse ApiHandler::getConfig() {
  const DarknessConfig& c = _det.config();
  JsonDocument doc;
  doc["dark_threshold_pct"] = c.darkThresholdPct;
  doc["hysteresis_pct"] = c.hysteresisPct;
  doc["confirm_time_s"] = c.confirmTimeMs / 1000;
  return jsonResponse(200, doc);
}

ApiResponse ApiHandler::postConfig(const String& body) {
  JsonDocument doc;
  if (body.length() > MAX_BODY || deserializeJson(doc, body)) {
    return errorResponse(400, "invalid_json", "Body must be valid JSON (max 256 bytes)");
  }

  DarknessConfig c = _det.config();   // start from current values (partial update)
  bool any = false;

  if (!doc["dark_threshold_pct"].isNull()) {
    if (!doc["dark_threshold_pct"].is<float>())
      return errorResponse(400, "invalid_value", "dark_threshold_pct must be a number");
    c.darkThresholdPct = doc["dark_threshold_pct"].as<float>(); any = true;
  }
  if (!doc["hysteresis_pct"].isNull()) {
    if (!doc["hysteresis_pct"].is<float>())
      return errorResponse(400, "invalid_value", "hysteresis_pct must be a number");
    c.hysteresisPct = doc["hysteresis_pct"].as<float>(); any = true;
  }
  if (!doc["confirm_time_s"].isNull()) {
    if (!doc["confirm_time_s"].is<float>())
      return errorResponse(400, "invalid_value", "confirm_time_s must be a number");
    float v = doc["confirm_time_s"].as<float>();
    if (v < 1 || v > 600 || v != floorf(v))
      return errorResponse(400, "invalid_value", "confirm_time_s must be an integer from 1 to 600");
    c.confirmTimeMs = (uint32_t)v * 1000UL; any = true;
  }

  if (!any) return errorResponse(400, "invalid_value", "No known config fields in request");
  if (!DarknessDetector::isValid(c)) {
    return errorResponse(400, "invalid_value",
      "Need 1<=threshold<=99, hysteresis>=1, threshold+hysteresis<=100");
  }
  _det.setConfig(c);
  _store.save(c);
  return getConfig();
}