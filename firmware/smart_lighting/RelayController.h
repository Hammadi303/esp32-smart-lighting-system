#pragma once
#include <Arduino.h>

class RelayController {
public:
  RelayController(uint8_t pin, bool activeLow);

  void begin();            // configures the pin and forces the relay OFF
  void set(bool on);       // command the relay
  bool isOn() const;       // actual GPIO output level, converted by polarity

private:
  uint8_t _pin;
  bool    _activeLow;
  uint8_t onLevel()  const { return _activeLow ? LOW  : HIGH; }
  uint8_t offLevel() const { return _activeLow ? HIGH : LOW;  }
};