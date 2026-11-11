#include "ConfigStore.h"
#include <Preferences.h>

static const char* NAMESPACE = "smartlight";

void ConfigStore::load(DarknessConfig& cfg) {
  Preferences prefs;
  prefs.begin(NAMESPACE, false);
  DarknessConfig c;
  c.darkThresholdPct = prefs.getFloat("dark_pct",   cfg.darkThresholdPct);
  c.hysteresisPct    = prefs.getFloat("hyst_pct",   cfg.hysteresisPct);
  c.confirmTimeMs    = prefs.getUInt ("confirm_ms", cfg.confirmTimeMs);
  prefs.end();
  if (DarknessDetector::isValid(c)) cfg = c;   // ignore corrupt values
}

void ConfigStore::save(const DarknessConfig& cfg) {
  Preferences prefs;
  prefs.begin(NAMESPACE, false);
  prefs.putFloat("dark_pct",   cfg.darkThresholdPct);
  prefs.putFloat("hyst_pct",   cfg.hysteresisPct);
  prefs.putUInt ("confirm_ms", cfg.confirmTimeMs);
  prefs.end();
}