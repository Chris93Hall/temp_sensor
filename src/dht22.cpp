#include "dht22.h"

void DHT22::begin() {
  pinMode(_pin, INPUT_PULLUP);
}

uint32_t DHT22::pulseLength(uint8_t level, uint32_t timeoutUs) {
  uint32_t start = micros();
  while (digitalRead(_pin) == level) {
    if (micros() - start > timeoutUs) return TIMEOUT;
  }
  return micros() - start;
}

DHT22::Status DHT22::read(float &temperatureC, float &humidity) {
  uint8_t data[5] = {0, 0, 0, 0, 0};
  uint32_t lowTimes[40], highTimes[40];
  bool timeout = false;

  // Start signal: host pulls low >= 1 ms, then releases.
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  delay(2);
  digitalWrite(_pin, HIGH);
  delayMicroseconds(30);
  pinMode(_pin, INPUT_PULLUP);

  // The bit stream is timing critical; block interrupts (~5 ms).
  noInterrupts();
  // Sensor response: ~80 us low, ~80 us high.
  if (pulseLength(HIGH, 100) == TIMEOUT) timeout = true;  // wait for line to drop
  if (!timeout && pulseLength(LOW, 120) == TIMEOUT) timeout = true;
  if (!timeout && pulseLength(HIGH, 120) == TIMEOUT) timeout = true;
  // 40 bits: each is ~50 us low then a high of ~27 us (0) or ~70 us (1).
  for (int i = 0; i < 40 && !timeout; i++) {
    lowTimes[i] = pulseLength(LOW, 100);
    highTimes[i] = pulseLength(HIGH, 120);
    if (lowTimes[i] == TIMEOUT || highTimes[i] == TIMEOUT) timeout = true;
  }
  interrupts();

  if (timeout) return ERR_TIMEOUT;

  for (int i = 0; i < 40; i++) {
    data[i / 8] <<= 1;
    if (highTimes[i] > lowTimes[i]) data[i / 8] |= 1;
  }

  if (((data[0] + data[1] + data[2] + data[3]) & 0xFF) != data[4]) return ERR_CHECKSUM;

  humidity = ((data[0] << 8) | data[1]) / 10.0f;
  int16_t raw = ((data[2] & 0x7F) << 8) | data[3];
  temperatureC = raw / 10.0f;
  if (data[2] & 0x80) temperatureC = -temperatureC;  // sign bit
  return OK;
}

const char *DHT22::statusText(Status s) {
  switch (s) {
    case OK: return "ok";
    case ERR_TIMEOUT: return "timeout (check wiring)";
    case ERR_CHECKSUM: return "checksum error";
  }
  return "unknown";
}
