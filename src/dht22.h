#pragma once
#include <Arduino.h>

// Minimal bit-banged driver for the DHT22 / AM2302 single-wire sensor.
class DHT22 {
public:
  enum Status { OK, ERR_TIMEOUT, ERR_CHECKSUM };

  explicit DHT22(uint8_t pin) : _pin(pin) {}
  void begin();

  // Reads the sensor. On OK, fills temperatureC (degrees C) and humidity (% RH).
  Status read(float &temperatureC, float &humidity);

  static const char *statusText(Status s);

private:
  static const uint32_t TIMEOUT = 0xFFFFFFFF;
  // Measures how long (us) the line stays at `level`; returns TIMEOUT on timeout.
  uint32_t pulseLength(uint8_t level, uint32_t timeoutUs);
  uint8_t _pin;
};
