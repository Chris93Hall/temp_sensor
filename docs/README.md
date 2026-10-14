# Source code documentation

Firmware is C++ using the Arduino framework for ESP8266, built with PlatformIO (`platformio.ini` points `src_dir` at `src/`).

## Files

| File | Purpose |
|------|---------|
| `src/main.cpp` | Setup (WiFi, NTP, web server) and main loop: serves HTTP, polls the sensor every `sensor.read_interval_ms`, reconnects WiFi if dropped, logs NTP status to serial until synced |
| `src/dht22.h/.cpp` | Dependency-free bit-banged DHT22/AM2302 driver |
| `src/web_api.h/.cpp` | HTTP server and endpoints; holds the latest `Reading` |
| `config/config.json` | WiFi credentials, IP address, port, sensor pin, read interval, NTP servers (JSON, compiled in at build time) |
| `scripts/load_config.py` | PlatformIO pre-build script that turns `config.json` into compile-time defines |
| `platformio.ini` | PlatformIO config: sources in `src/`, output in `build/`, runs `load_config.py` |
| `Dockerfile`, `Makefile` | Containerised build environment and its `make` targets |

## Configuration (`config/config.json`)

| Key | Define | Meaning |
|-----|--------|---------|
| `label` | `DEVICE_LABEL` | Name for this sensor, shown on the web page and in `/api/data` |
| `wifi.ssid`, `wifi.password` | `WIFI_SSID`, `WIFI_PASSWORD` | WiFi credentials |
| `network.use_static_ip` | `USE_STATIC_IP` | `true` = fixed address, `false` = DHCP |
| `network.ip`, `gateway`, `subnet`, `dns` | `STATIC_IP`, `STATIC_GATEWAY`, `STATIC_SUBNET`, `STATIC_DNS` | Static addressing (parsed at boot) |
| `network.port` | `WEB_PORT` | HTTP port |
| `sensor.pin` | `DHT_PIN` | GPIO number of the DHT22 data line |
| `sensor.read_interval_ms` | `SENSOR_READ_INTERVAL_MS` | Time between sensor reads (DHT22 needs ≥ 2000) |
| `ntp_servers` | `NTP_SERVER_1..3` | 1–3 NTP servers (primary, then fallbacks); unused slots are empty strings |

The values are fixed at compile time. Changing them requires a rebuild and re-flash.

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
`timeSynced` is true once the system clock is past 2020 (set by NTP); the timestamp is taken when the sensor is read, not when the request arrives.
On a failed read, the previous good reading is kept and `lastError` is updated (shown only if no good reading exists).

## HTTP API

Served on `network.port` (default 80) at `network.ip` in `config/config.json`.

### `GET /api/data`
```json
{"label":"Living room","temperature_c":22.4,"temperature_f":72.3,"humidity_percent":41.7,
 "timestamp":"2026-10-06T14:03:11Z","timestamp_unix":1791295391}
```
`label` is the `label` value from `config.json` (empty string if unset). `timestamp` is ISO 8601 UTC; `timestamp_unix` is seconds since 1970. Both are `null` until NTP has synced.
Also includes `Access-Control-Allow-Origin: *`. Returns HTTP 503 with `{"error":"..."}` if no valid reading exists yet.

### `GET /`
Basic HTML page (the label is shown under the title) with the same data (auto-refreshes every 10 s) and links to the JSON endpoints.

### `GET /api/uptime`
```json
{"uptime_seconds":3725,"uptime_human":"0d 1h 2m 5s"}
```
Tracked as a 64-bit millisecond counter that accounts for `millis()` wrapping every ~49.7 days, so it no longer rolls over in practice (centuries). It resets to 0 on reboot or power loss.

### `POST /api/reboot`
Restarts the board. Responds `{"status":"rebooting"}` and reboots about 0.5 s later. Other methods (e.g. GET) get 405,
so browsers, link prefetchers and crawlers can't trigger it. There is no authentication: anyone on the network can reboot the board.
```
curl -X POST http://192.168.1.50/api/reboot
```

Unknown paths return 404 JSON.
