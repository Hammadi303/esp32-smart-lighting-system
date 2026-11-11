#pragma once
#include <Arduino.h>

// 1 = Wokwi simulation build, 0 = real hardware
#define TARGET_WOKWI 0

// Simulation-only test aid. Must stay 0 on hardware.
#define SIM_INJECT_NOISE 0
constexpr int SIM_NOISE_AMPLITUDE = 60;

// ---- GPIO map ----
constexpr uint8_t PIN_LDR        = 34;
constexpr uint8_t PIN_RELAY      = 27;
// LoRa pins: reserved, unused (LoRa was dropped from the scope)
constexpr uint8_t PIN_LORA_SCK   = 18;
constexpr uint8_t PIN_LORA_MISO  = 19;
constexpr uint8_t PIN_LORA_MOSI  = 23;
constexpr uint8_t PIN_LORA_NSS   = 4;
constexpr uint8_t PIN_LORA_RST   = 25;
constexpr uint8_t PIN_LORA_DIO0  = 26;

// ---- Relay ----
// Wokwi relay module: active-HIGH (verified by test). Real module: check yours.
constexpr bool RELAY_ACTIVE_LOW = !TARGET_WOKWI;

// ---- Wi-Fi ----
#if TARGET_WOKWI
  constexpr const char* WIFI_SSID = "Wokwi-GUEST";
  constexpr const char* WIFI_PASS = "";
  constexpr uint8_t WIFI_CHANNEL = 6;
#else
  constexpr const char* WIFI_SSID = "YOUR_WIFI_NAME";
  constexpr const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
  constexpr uint8_t WIFI_CHANNEL = 0;
#endif
constexpr uint32_t WIFI_RETRY_MS = 10000;
constexpr uint16_t HTTP_PORT     = 80;

// ---- Sensor timing ----
constexpr uint32_t SAMPLE_PERIOD_MS = 100;
constexpr uint32_t PRINT_PERIOD_MS  = 2000;

// ---- Brightness calibration (re-measure on real hardware) ----
constexpr uint16_t ADC_BRIGHT_REF = 200;
constexpr uint16_t ADC_DARK_REF   = 3500;

// ---- Filter ----
constexpr uint8_t FILTER_WINDOW = 10;

// ---- Darkness detection defaults ----
constexpr float    DEFAULT_DARK_THRESHOLD_PCT = 30.0f;
constexpr float    DEFAULT_HYSTERESIS_PCT     = 10.0f;
constexpr uint32_t DEFAULT_CONFIRM_TIME_MS    = 10000;