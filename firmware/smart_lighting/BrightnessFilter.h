#pragma once
#include <Arduino.h>

// Moving-average filter over the last N samples (O(1) per update).
class BrightnessFilter {
public:
  static constexpr uint8_t MAX_WINDOW = 32;

  explicit BrightnessFilter(uint8_t windowSize);

  void  add(uint16_t sample);     // insert a new sample
  float value() const;            // current mean (0 if empty)
  bool  full() const { return _count == _size; }
  void  reset();

private:
  uint16_t _buf[MAX_WINDOW] = {0};
  uint8_t  _size;
  uint8_t  _head  = 0;   // next position to overwrite
  uint8_t  _count = 0;   // samples currently stored
  uint32_t _sum   = 0;   // running sum (integer: no drift)
};