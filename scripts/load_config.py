"""PlatformIO pre-build script: turns config/config.json into compile-time defines."""
import json
import os

Import("env")  # noqa: F821  (provided by PlatformIO/SCons)

path = os.path.join(env["PROJECT_DIR"], "config", "config.json")  # noqa: F821
with open(path) as f:
    cfg = json.load(f)

net = cfg["network"]
defines = [
    ("WIFI_SSID", env.StringifyMacro(cfg["wifi"]["ssid"])),  # noqa: F821
    ("WIFI_PASSWORD", env.StringifyMacro(cfg["wifi"]["password"])),  # noqa: F821
    ("USE_STATIC_IP", int(bool(net["use_static_ip"]))),
    ("STATIC_IP", env.StringifyMacro(net["ip"])),  # noqa: F821
    ("STATIC_GATEWAY", env.StringifyMacro(net["gateway"])),  # noqa: F821
    ("STATIC_SUBNET", env.StringifyMacro(net["subnet"])),  # noqa: F821
    ("STATIC_DNS", env.StringifyMacro(net["dns"])),  # noqa: F821
    ("WEB_PORT", int(net["port"])),
    ("DHT_PIN", int(cfg["sensor"]["pin"])),
    ("SENSOR_READ_INTERVAL_MS", int(cfg["sensor"]["read_interval_ms"])),
]

# configTime() takes at most 3 servers; unused slots are empty strings.
ntp = cfg["ntp_servers"]
if not 1 <= len(ntp) <= 3:
    raise ValueError("config.json: ntp_servers must list 1 to 3 servers")
for i in range(3):
    defines.append(("NTP_SERVER_%d" % (i + 1), env.StringifyMacro(ntp[i] if i < len(ntp) else "")))  # noqa: F821

env.Append(CPPDEFINES=defines)  # noqa: F821
