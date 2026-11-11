#pragma once
#include <Arduino.h>
#include "DarknessDetector.h"
#include "RelayController.h"
#include "WiFiManager.h"
#include "ConfigStore.h"

// Latest measurements, filled in by the main loop.
struct SensorData {
  uint16_t raw = 0;
  float    filtered = 0;
  float    brightnessPct = 0;
};

struct ApiResponse {
  int    status;
  String body;     // JSON text
};

// Transport-independent REST logic: (method, path, body) -> (status, JSON).
class ApiHandler {
public:
  ApiHandler(DarknessDetector& det, RelayController& relay, WiFiManager& wifi,
             ConfigStore& store, const SensorData& sensor);

  ApiResponse handle(const String& method, const String& path,
                     const String& body, uint32_t nowMs);
  void setLoraInitialized(bool v) { _loraInit = v; }

private:
  ApiResponse getStatus(uint32_t nowMs);
  ApiResponse postLight(const String& body);
  ApiResponse getConfig();
  ApiResponse postConfig(const String& body);

  DarknessDetector& _det;
  RelayController&  _relay;
  WiFiManager&      _wifi;
  ConfigStore&      _store;
  const SensorData& _sensor;
  bool _loraInit = false;
};