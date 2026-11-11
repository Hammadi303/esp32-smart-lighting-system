#include "BrightnessFilter.h"

BrightnessFilter::BrightnessFilter(uint8_t windowSize)
  : _size(constrain(windowSize, 1, MAX_WINDOW)) {}

void BrightnessFilter::add(uint16_t sample) {
  if (_count == _size) {
    _sum -= _buf[_head];        // window full: drop the oldest sample
  } else {
    _count++;                   // still filling: average over what we have
  }
  _buf[_head] = sample;
  _sum += sample;
  _head = (_head + 1) % _size;
}

float BrightnessFilter::value() const {
  return _count ? (float)_sum / (float)_count : 0.0f;
}

void BrightnessFilter::reset() {
  _head = 0; _count = 0; _sum = 0;
}