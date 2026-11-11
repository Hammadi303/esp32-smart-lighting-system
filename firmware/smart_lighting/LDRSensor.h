#pragma once
#include <Arduino.h>

class LDRSensor {
public:
  LDRSensor(uint8_t pin, uint16_t adcBright, uint16_t adcDark);

  void begin();                       // configure ADC
  bool update(uint32_t nowMs);        // true when a new sample was taken
  uint16_t raw() const { return _raw; }
  float toBrightnessPct(float adc) const;  // relative brightness 0..100

private:
  uint8_t  _pin;
  uint16_t _adcBright, _adcDark;
  uint16_t _raw = 0;
  uint32_t _lastSampleMs = 0;
};