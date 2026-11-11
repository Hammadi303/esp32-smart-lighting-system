#include "DarknessDetector.h"

bool DarknessDetector::isValid(const DarknessConfig& c) {
  return c.darkThresholdPct >= 1.0f && c.darkThresholdPct <= 99.0f &&
         c.hysteresisPct    >= 1.0f &&
         (c.darkThresholdPct + c.hysteresisPct) <= 100.0f &&
         c.confirmTimeMs >= 1000 && c.confirmTimeMs <= 600000;
}

bool DarknessDetector::setConfig(const DarknessConfig& c) {
  if (!isValid(c)) return false;   // reject, keep the old settings
  _cfg = c;
  return true;
}

bool DarknessDetector::update(float pct, uint32_t nowMs) {
  const DarkState previous = _state;
  const float tDark   = _cfg.darkThresholdPct;
  const float tBright = tDark + _cfg.hysteresisPct;

  switch (_state) {
    case DarkState::BRIGHT:
      if (pct < tDark) {
        _state = DarkState::DARK_PENDING;
        _pendingStartMs = nowMs;
      }
      break;

    case DarkState::DARK_PENDING:
      if (pct >= tDark) {
        _state = DarkState::BRIGHT;                       // dip was too short
      } else if (nowMs - _pendingStartMs >= _cfg.confirmTimeMs) {
        _state = DarkState::DARK_CONFIRMED;
        _eventId++;                                       // one event per episode
        _eventStartMs = nowMs;
      }
      break;

    case DarkState::DARK_CONFIRMED:
      if (pct >= tBright) _state = DarkState::BRIGHT;     // hysteresis exit
      break;
  }
  return _state != previous;
}

const char* DarknessDetector::stateName(DarkState s) {
  switch (s) {
    case DarkState::BRIGHT:         return "BRIGHT";
    case DarkState::DARK_PENDING:   return "DARK_PENDING";
    case DarkState::DARK_CONFIRMED: return "DARK_CONFIRMED";
  }
  return "?";
}