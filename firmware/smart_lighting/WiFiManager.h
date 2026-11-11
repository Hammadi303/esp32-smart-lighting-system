#pragma once
#include <Arduino.h>

// Non-blocking Wi-Fi station with periodic reconnect.
class WiFiManager {
public:
  void   begin(const char* ssid, const char* pass, uint8_t channel);
  void   update(uint32_t nowMs);          // call every loop()
  bool   connected() const;
  String ip() const;                      // "" when not connected
  int    rssi() const;

private:
  const char* _ssid = nullptr;
  const char* _pass = nullptr;
  uint8_t  _channel = 0;
  uint32_t _lastAttemptMs = 0;
  bool     _wasConnected = false;
};