#pragma once
#include <Arduino.h>

enum class DarkState : uint8_t { BRIGHT, DARK_PENDING, DARK_CONFIRMED };

struct DarknessConfig {
  float    darkThresholdPct;   // enter dark below this
  float    hysteresisPct;      // return to bright at threshold + hysteresis
  uint32_t confirmTimeMs;      // darkness must persist this long
};

class DarknessDetector {
public:
  static bool isValid(const DarknessConfig& c);

  bool setConfig(const DarknessConfig& c);        // false if invalid (unchanged)
  const DarknessConfig& config() const { return _cfg; }

  // Call with the latest FILTERED brightness. Returns true if the state changed.
  bool update(float brightnessPct, uint32_t nowMs);

  DarkState state() const { return _state; }
  bool      isDark() const { return _state == DarkState::DARK_CONFIRMED; }
  uint32_t  eventId() const { return _eventId; }          // 0 = no event yet
  uint32_t  eventStartMs() const { return _eventStartMs; }
  static const char* stateName(DarkState s);

private:
  DarknessConfig _cfg = {30.0f, 10.0f, 10000};
  DarkState _state = DarkState::BRIGHT;
  uint32_t  _pendingStartMs = 0;
  uint32_t  _eventId = 0;
  uint32_t  _eventStartMs = 0;
};