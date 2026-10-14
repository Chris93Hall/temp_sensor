#pragma once
#include <Arduino.h>

// Latest sensor reading shared between the sensor loop and the web handlers.
struct Reading {
  bool valid = false;       // false until the first successful read
  float temperatureC = 0;
  float humidity = 0;
  time_t unixTime = 0;      // seconds since 1970 (0 if clock not synced)
  bool timeSynced = false;
  uint32_t millisAtRead = 0;
  const char *lastError = "no reading yet";
};

void webApiBegin(uint16_t port);
void webApiHandle();
void webApiUpdate(const Reading &r);
