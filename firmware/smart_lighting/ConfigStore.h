#pragma once
#include "DarknessDetector.h"

// Persists the darkness settings in flash (NVS).
class ConfigStore {
public:
  void load(DarknessConfig& cfg);          // cfg keeps its defaults if nothing valid is stored
  void save(const DarknessConfig& cfg);
};