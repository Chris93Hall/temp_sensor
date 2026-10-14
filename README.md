# ESP8266 Temperature & Humidity Monitor

ESP-12F (ESP8266) + DHT22/AM2302 serving temperature and humidity over WiFi.

```
src/            firmware source (C++, Arduino framework)
config/         user configuration (config.json)
scripts/        build helper that loads config.json
docs/           code and HTTP API documentation
datasheets/     datasheets for the chip, module, sensor and NodeMCU board
WIRING.md       how to connect the sensor (bare ESP-12F and NodeMCU)
platformio.ini  PlatformIO build configuration
Dockerfile      build environment (PlatformIO + ESP8266 toolchain)
Makefile        docker-build / docker-run / build / upload / clean
build/          compiled firmware (generated, git-ignored)
```

## Wiring

See [WIRING.md](WIRING.md) for how to connect the DHT22 to a bare ESP-12F or a NodeMCU board (DATA → GPIO4 / D2, 3V3, GND, 10 kΩ pull-up).

## Configure

Edit `config/config.json`. It is read at build time by `scripts/load_config.py` and compiled into the firmware, so rebuild and re-flash after changing it. The WiFi password is stored in plain text in the firmware binary and in this file; don't commit or share either with real credentials.


- `label` — a name for this sensor (e.g. "Living room"), shown on the web page and included in the `/api/data` JSON
- `wifi.ssid`, `wifi.password`
- `network.ip` — the IP address the web API serves from (with `gateway`, `subnet`, `dns`).
  Set `network.use_static_ip` to `false` to use DHCP instead (the address is printed on the serial monitor).
- `network.port`, `sensor.pin`, `sensor.read_interval_ms`, `ntp_servers` (list of 1–3 NTP servers; later ones are fallbacks, and an IP address works even if DNS is broken)

The first NTP server is the primary; the others are fallbacks. Pick a static IP outside your router's DHCP range.

## Build and upload

### With Docker (no local toolchain needed)

```
make docker-build   # once: build the image (downloads the ESP8266 toolchain)
make build          # compile; output is build/esp12e/firmware.bin
make upload         # flash the board (PORT=/dev/ttyUSB0 by default, e.g. make upload PORT=/dev/ttyUSB1)
make docker-run     # interactive shell in the container with the project mounted
make clean          # delete build/ and .pio/
```

`make help` lists the targets. Serial output needs the monitor to run on the machine the board is plugged into (see below).

### Locally

1. Install [PlatformIO Core](https://platformio.org/install/cli) (`pip install platformio`).
2. Connect the board over USB-serial. For a bare ESP-12F, hold GPIO0 low while resetting to enter flash mode.
3. From the project root:
   ```
   pio run                    # compile (output in build/esp12e/firmware.bin)
   pio run -t upload          # compile and flash
   pio device monitor         # serial output at 115200 baud
   ```
   Add `--upload-port /dev/ttyUSB0` if auto-detect picks the wrong port. On Linux you may need to add your user to the `dialout` group.

The serial monitor shows WiFi progress, the IP address, each sensor reading, and NTP status while the clock is not yet synced.

## Use

```
curl http://192.168.1.50/api/data
curl http://192.168.1.50/api/uptime
curl -X POST http://192.168.1.50/api/reboot   # restart the board (no authentication)
```
Open `http://192.168.1.50/` in a browser for the HTML page. Replace the address with your `network.ip`. See [docs/README.md](docs/README.md) for the full API and code details.

## Troubleshooting

- `timeout (check wiring)` in the serial log: check VCC/GND/DATA and the pull-up.
- Timestamps `null`: the clock hasn't synced. The serial monitor prints `Waiting for NTP... resolve ...: FAILED` if DNS is wrong; check `network.gateway` and `network.dns`, or put an IP address in `ntp_servers`. The board needs internet access and UDP port 123.
- Timestamps are UTC.
- Can't reach the board: check that `network.ip` is on your subnet and unused.
