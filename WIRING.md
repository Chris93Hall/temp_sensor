# Wiring

DHT22/AM2302 (pins left to right, facing the grid side):

| DHT22 pin | Connect to ESP-12F |
|-----------|--------------------|
| 1 VCC     | 3V3                |
| 2 DATA    | GPIO4 (IO4) |
| 3 NC      | not connected      |
| 4 GND     | GND                |

- Put a **10 kΩ pull-up resistor** between DATA and VCC (many 3-pin breakout boards already have one).
- Power the sensor from 3.3 V, not 5 V, so the data line is safe for the ESP8266.
- On NodeMCU-style boards GPIO4 is labelled **D2**. Change `sensor.pin` in `config/config.json` to use another pin.

A bare ESP-12F also needs: 3V3 and GND, EN and RST pulled high (10 kΩ to 3V3), GPIO15 pulled low (10 kΩ to GND),
and for flashing GPIO0 pulled low at reset (GPIO0 → GND). Use a breakout/adapter board or a NodeMCU/Wemos-style board
to avoid wiring all this yourself. Power must supply ≥ 300 mA peak.

## Using a NodeMCU board

A NodeMCU DevKit (ESP-12E/F module + USB-UART bridge + 3.3 V regulator) needs no extra wiring beyond the sensor, and flashes over USB with no GPIO0/reset fiddling.

| DHT22 pin | NodeMCU pin |
|-----------|-------------|
| 1 VCC     | 3V3         |
| 2 DATA    | D2 (GPIO4)  |
| 3 NC      | not connected |
| 4 GND     | GND         |

- Add a 10 kΩ resistor between DATA (D2) and 3V3 unless your DHT22 is on a breakout board that has one.
- D2 is the default (`sensor.pin` = 4 in `config/config.json`). Other labels map to GPIOs as D1 = 5, D5 = 14, D6 = 12, D7 = 13. Avoid D3 (GPIO0), D4 (GPIO2) and D8 (GPIO15): they are boot-strapping pins and can stop the board booting if the sensor pulls them the wrong way.
- Power and flash with a USB data cable (not charge-only). The board appears as `/dev/ttyUSB0` (CP2102 chip) or `/dev/ttyACM0`; pass it with `make upload PORT=/dev/ttyUSB0`.
- The sensor can also be powered from the board's 3V3 pin because the data line is then at 3.3 V. Do not use VIN (5 V) for the sensor.
- Pin map and schematic: see `datasheets/NodeMCU_DevKit_v1.0_pinmap.png` and `datasheets/NodeMCU_DevKit_v1.0_schematic.pdf`.
- Boards with a CH340 instead of CP2102 work the same; you may need the CH340 driver on Windows/macOS.
