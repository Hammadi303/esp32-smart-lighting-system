
#include "config.h"
#include "LDRSensor.h"
#include "BrightnessFilter.h"
#include "DarknessDetector.h"
#include "RelayController.h"
#include "ConfigStore.h"
#include "WiFiManager.h"
#include "ApiHandler.h"
#include "ApiServer.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

LDRSensor        ldr(PIN_LDR, ADC_BRIGHT_REF, ADC_DARK_REF);
BrightnessFilter filter(FILTER_WINDOW);
DarknessDetector detector;
RelayController  relay(PIN_RELAY, RELAY_ACTIVE_LOW);
ConfigStore      configStore;
WiFiManager      wifi;
SensorData       sensorData;
ApiHandler       api(detector, relay, wifi, configStore, sensorData);
ApiServer        server(api, HTTP_PORT);

uint16_t lastSample = 0;
float    brightnessPct = 0;
uint32_t lastPrintMs = 0;
String   cmdBuffer;

void printRelay() {
  Serial.printf("Relay: %s\n", relay.isOn() ? "ON" : "OFF");
}

// Serial commands: on | off | status
// api <GET|POST> <path> [json body]
void executeCommand(String cmd) {
  cmd.trim();

  if (cmd.equalsIgnoreCase("on")) {
    relay.set(true);
    Serial.print("Command accepted. ");
    printRelay();
  }
  else if (cmd.equalsIgnoreCase("off")) {
    relay.set(false);
    Serial.print("Command accepted. ");
    printRelay();
  }
  else if (cmd.equalsIgnoreCase("status")) {
    printRelay();
  }
  else if (cmd.startsWith("api ")) {
    String rest = cmd.substring(4);
    int s1 = rest.indexOf(' ');

    if (s1 < 0) {
      Serial.println("ERR: usage: api <GET|POST> <path> [json]");
      return;
    }

    String method = rest.substring(0, s1);
    method.toUpperCase();

    String rest2 = rest.substring(s1 + 1);
    int s2 = rest2.indexOf(' ');

    String path = (s2 < 0) ? rest2 : rest2.substring(0, s2);
    String body = (s2 < 0) ? String("") : rest2.substring(s2 + 1);

    ApiResponse r = api.handle(method, path, body, millis());
    Serial.printf("HTTP %d\n%s\n", r.status, r.body.c_str());
  }
  else {
    Serial.printf("ERR: invalid command '%s'\n", cmd.c_str());
  }
}

void handleSerialInput() {
  while (Serial.available()) {
    char c = (char)Serial.read();

    if (c == '\n' || c == '\r') {
      if (cmdBuffer.length()) {
        executeCommand(cmdBuffer);
        cmdBuffer = "";
      }
    }
    else if (cmdBuffer.length() < 200) {
      cmdBuffer += c;
    }
    else {
      cmdBuffer = "";
      Serial.println("ERR: command too long");
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize the onboard LED pin.
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  relay.begin();
  ldr.begin();

  DarknessConfig cfg = {
    DEFAULT_DARK_THRESHOLD_PCT,
    DEFAULT_HYSTERESIS_PCT,
    DEFAULT_CONFIRM_TIME_MS
  };

  configStore.load(cfg);
  detector.setConfig(cfg);

  Serial.println("Smart Lighting - Milestone 5: Wi-Fi + REST API");

  Serial.printf(
    "Config: threshold=%.0f%% hysteresis=%.0f%% confirm=%lus\n",
    cfg.darkThresholdPct,
    cfg.hysteresisPct,
    (unsigned long)(cfg.confirmTimeMs / 1000)
  );

  Serial.print("Boot state. ");
  printRelay();

  wifi.begin(WIFI_SSID, WIFI_PASS, WIFI_CHANNEL);
}

void loop() {
  const uint32_t now = millis();

  wifi.update(now);
  server.update(wifi.connected());
  handleSerialInput();

  // Read and process the LDR sensor.
  if (ldr.update(now)) {
    lastSample = ldr.raw();

#if SIM_INJECT_NOISE
    lastSample = constrain(
      (int)lastSample +
      random(-SIM_NOISE_AMPLITUDE, SIM_NOISE_AMPLITUDE + 1),
      0,
      4095
    );
#endif

    filter.add(lastSample);
    brightnessPct = ldr.toBrightnessPct(filter.value());

    sensorData = {
      lastSample,
      filter.value(),
      brightnessPct
    };

    if (detector.update(brightnessPct, now)) {
      Serial.printf(
        "[%lu ms] state -> %s (event_id=%lu)\n",
        now,
        DarknessDetector::stateName(detector.state()),
        (unsigned long)detector.eventId()
      );

      if (detector.state() == DarkState::DARK_CONFIRMED) {
        Serial.println(
          "Darkness confirmed. Lamp unchanged, waiting for user command."
        );
      }
    }
  }

  // Print system status every 2 seconds.
  if (now - lastPrintMs >= PRINT_PERIOD_MS) {
    lastPrintMs = now;

    Serial.printf(
      "raw:%u,filtered:%.0f,brightness_pct:%.1f,state:%s,event:%lu,relay:%s\n",
      lastSample,
      filter.value(),
      brightnessPct,
      DarknessDetector::stateName(detector.state()),
      (unsigned long)detector.eventId(),
      relay.isOn() ? "ON" : "OFF"
    );
  }
}
