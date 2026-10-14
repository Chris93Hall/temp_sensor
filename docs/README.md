# Source code documentation

Firmware is C++ using the Arduino framework for ESP8266, built with PlatformIO (`platformio.ini` points `src_dir` at `src/`).

## Files

| File | Purpose |
|------|---------|
| `src/main.cpp` | Setup (WiFi, NTP, web server) and main loop (poll sensor every `sensor.read_interval_ms`) |
| `src/dht22.h/.cpp` | Dependency-free bit-banged DHT22/AM2302 driver |
| `src/web_api.h/.cpp` | HTTP server and endpoints; holds the latest `Reading` |
| `config/config.json` | WiFi credentials, IP address, port, sensor pin, NTP server |
| `scripts/load_config.py` | PlatformIO pre-build script that turns `config.json` into compile-time defines |

## `DHT22` class

```cpp
DHT22 dht(4);          // GPIO number
dht.begin();
float t, h;
DHT22::Status s = dht.read(t, h);   // OK, ERR_TIMEOUT or ERR_CHECKSUM
```

Protocol: the host pulls the data line low for 2 ms, releases it, and the sensor answers with an 80 µs low / 80 µs high
preamble followed by 40 bits (16 humidity, 16 temperature, 8 checksum). Each bit starts with a ~50 µs low; the following
high lasts ~27 µs for `0` and ~70 µs for `1`. The driver measures each pulse and compares high vs. low length, so it is
tolerant of clock differences. Interrupts are disabled for the ~5 ms read. Humidity and temperature are in tenths
(temperature's top bit is the sign). Reads must be at least 2 s apart.

## `Reading` struct (`web_api.h`)

Fields: `valid`, `temperatureC`, `humidity`, `unixTime`, `timeSynced`, `millisAtRead`, `lastError`.
On a failed read, the previous good reading is kept and `lastError` is updated (shown only if no good reading exists).

## HTTP API

Served on `network.port` (default 80) at `network.ip` in `config/config.json`.

### `GET /api/data`
```json
{"temperature_c":22.4,"temperature_f":72.3,"humidity_percent":41.7,
 "timestamp":"2026-10-06T14:03:11Z","timestamp_unix":1791295391}
```
`timestamp` is ISO 8601 UTC; `timestamp_unix` is seconds since 1970. Both are `null` until NTP has synced.
Returns HTTP 503 with `{"error":"..."}` if no valid reading exists yet.

### `GET /`
Basic HTML page with the same data (auto-refreshes every 10 s).

### `GET /api/uptime`
```json
{"uptime_seconds":3725,"uptime_human":"0d 1h 2m 5s"}
```
Based on `millis()`, which wraps after ~49.7 days.

### `POST /api/reboot`
Restarts the board. Responds `{"status":"rebooting"}` and reboots about 0.5 s later. Other methods (e.g. GET) get 405,
so browsers, link prefetchers and crawlers can't trigger it. There is no authentication: anyone on the network can reboot the board.
```
curl -X POST http://192.168.1.50/api/reboot
```

Unknown paths return 404 JSON.
