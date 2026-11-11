#include "LDRSensor.h"
#include "config.h"

LDRSensor::LDRSensor(uint8_t pin, uint16_t adcBright, uint16_t adcDark)
  : _pin(pin), _adcBright(adcBright), _adcDark(adcDark) {}

void LDRSensor::begin() {
  analogReadResolution(12);                    // 0..4095
  analogSetPinAttenuation(_pin, ADC_11db);     // ~0..3.1 V input range
  pinMode(_pin, INPUT);
}

bool LDRSensor::update(uint32_t nowMs) {
  if (nowMs - _lastSampleMs < SAMPLE_PERIOD_MS) return false;  // not time yet
  _lastSampleMs = nowMs;   // unsigned subtraction stays correct across millis() overflow
  _raw = analogRead(_pin);
  return true;
}

float LDRSensor::toBrightnessPct(float adc) const {
  // Linear map: adcDark -> 0 %, adcBright -> 100 % (dark = high ADC)
  float pct = 100.0f * (float)(_adcDark - adc) / (float)(_adcDark - _adcBright);
  return constrain(pct, 0.0f, 100.0f);
}