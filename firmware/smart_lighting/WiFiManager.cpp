#include "WiFiManager.h"
#include "config.h"
#include <WiFi.h>

void WiFiManager::begin(const char* ssid, const char* pass, uint8_t channel) {
  _ssid = ssid; _pass = pass; _channel = channel;
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);                 // don't wear flash with credentials
  WiFi.begin(_ssid, _pass, _channel);     // returns immediately (non-blocking)
  _lastAttemptMs = millis();
  Serial.printf("Wi-Fi: connecting to '%s'...\n", _ssid);
}

void WiFiManager::update(uint32_t nowMs) {
  const bool up = (WiFi.status() == WL_CONNECTED);

  if (up && !_wasConnected) {
    Serial.printf("Wi-Fi: connected, IP %s\n", WiFi.localIP().toString().c_str());
  } else if (!up && _wasConnected) {
    Serial.println("Wi-Fi: connection lost");
  }
  _wasConnected = up;

  if (!up && nowMs - _lastAttemptMs >= WIFI_RETRY_MS) {
    _lastAttemptMs = nowMs;
    Serial.println("Wi-Fi: retrying...");
    WiFi.disconnect();
    WiFi.begin(_ssid, _pass, _channel);
  }
}

bool WiFiManager::connected() const { return WiFi.status() == WL_CONNECTED; }
String WiFiManager::ip() const { return connected() ? WiFi.localIP().toString() : String(""); }
int WiFiManager::rssi() const { return connected() ? WiFi.RSSI() : 0; }