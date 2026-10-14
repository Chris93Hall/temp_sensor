# ESP8266 Temperature & Humidity Monitor

ESP-12F (ESP8266) + DHT22/AM2302 serving temperature and humidity over WiFi.

```
WIRING.md     sensor wiring
src/          firmware source
scripts/      build helper (JSON config loader)        docs/       code documentation
config/       user configuration     datasheets/ component datasheets
platformio.ini  build configuration
```

## Wiring

See [WIRING.md](WIRING.md) for how to connect the DHT22 to the ESP-12F (DATA → GPIO4, 3V3, GND, 10 kΩ pull-up).

## Configure

Edit `config/config.json` (read at build time by `scripts/load_config.py`, so rebuild after changing it):

- `wifi.ssid`, `wifi.password`
- `network.ip` — the IP address the web API serves from (with `gateway`, `subnet`, `dns`).
  Set `network.use_static_ip` to `false` to use DHCP instead (the address is printed on the serial monitor).
- `network.port`, `sensor.pin`, `sensor.read_interval_ms`, `ntp_server`

Pick a static IP outside your router's DHCP range.

## Build and upload

With Docker (no local toolchain needed): `make docker-build`, then `make build`. `make help` lists all targets.
Or locally:

1. Install [PlatformIO Core](https://platformio.org/install/cli) (`pip install platformio`).
2. Connect the board over USB-serial. For a bare ESP-12F, hold GPIO0 low while resetting to enter flash mode.
3. From the project root:
   ```
   pio run                    # compile (output in build/esp12e/firmware.bin)
   pio run -t upload          # compile and flash
   pio device monitor         # serial output at 115200 baud
   ```
   Add `--upload-port /dev/ttyUSB0` if auto-detect picks the wrong port. On Linux you may need to add your user to the `dialout` group.

## Use

```
curl http://192.168.1.50/api/data
curl http://192.168.1.50/api/uptime
curl -X POST http://192.168.1.50/api/reboot   # restart the board
```
Open `http://192.168.1.50/` in a browser for the HTML page. See [docs/README.md](docs/README.md) for the full API and code details.

## Troubleshooting

- `timeout (check wiring)` in the serial log: check VCC/GND/DATA and the pull-up.
- Timestamps `null`: the board hasn't reached an NTP server yet (needs internet; check gateway/DNS settings).
- The board's reads are UTC.

> Note: this code was written without access to a PlatformIO toolchain, so it has not been compiled or run on hardware.
