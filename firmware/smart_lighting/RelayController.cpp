#include "RelayController.h"

RelayController::RelayController(uint8_t pin, bool activeLow)
  : _pin(pin), _activeLow(activeLow) {}

void RelayController::begin() {
  digitalWrite(_pin, offLevel());  // preload OFF level before enabling the driver
  pinMode(_pin, OUTPUT);           // pin now drives the OFF level, no ON glitch
  digitalWrite(_pin, offLevel());
}

void RelayController::set(bool on) {
  digitalWrite(_pin, on ? onLevel() : offLevel());
}

bool RelayController::isOn() const {
  return digitalRead(_pin) == onLevel();   // read back what the pin is really driving
}