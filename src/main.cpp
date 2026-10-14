#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include "dht22.h"
#include "web_api.h"

static DHT22 dht(DHT_PIN);
static Reading current;
static uint32_t lastRead = 0;
static uint32_t lastNtpLog = 0;
static bool ntpLogged = false;

static void connectWifi() {
  WiFi.mode(WIFI_STA);
#if USE_STATIC_IP
  IPAddress ip, gateway, subnet, dns;
  if (!ip.fromString(STATIC_IP) || !gateway.fromString(STATIC_GATEWAY) ||
      !subnet.fromString(STATIC_SUBNET) || !dns.fromString(STATIC_DNS)) {
    Serial.println("Invalid address in config/config.json");
  }
  WiFi.config(ip, gateway, subnet, dns);
#endif
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.print("\nConnected, serving on http://");
  Serial.println(WiFi.localIP());
}

static void sampleSensor() {
  float t, h;
  DHT22::Status s = dht.read(t, h);
  if (s == DHT22::OK) {
    current.valid = true;
    current.temperatureC = t;
    current.humidity = h;
    time_t now = time(nullptr);
    current.timeSynced = now > 1600000000;  // clock set once NTP succeeds
    current.unixTime = current.timeSynced ? now : 0;
    current.millisAtRead = millis();
    Serial.printf("T=%.1fC H=%.1f%%\n", t, h);
  } else {
    current.lastError = DHT22::statusText(s);
    Serial.printf("DHT22 read failed: %s\n", current.lastError);
    // keep the last good reading, if any
  }
  webApiUpdate(current);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  dht.begin();
  connectWifi();
  // UTC. Servers come from config.json (1-3; unused slots are empty strings).
  configTime(0, 0, NTP_SERVER_1, NTP_SERVER_2[0] ? NTP_SERVER_2 : nullptr,
             NTP_SERVER_3[0] ? NTP_SERVER_3 : nullptr);
  webApiBegin(WEB_PORT);
  delay(1000);  // let the DHT22 settle after power-up
  sampleSensor();
  lastRead = millis();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWifi();
  webApiHandle();
  if (!ntpLogged) {
    if (time(nullptr) > 1600000000) {
      ntpLogged = true;
      Serial.printf("NTP synced, unix time %lu\n", (unsigned long)time(nullptr));
    } else if (millis() - lastNtpLog >= 10000) {
      lastNtpLog = millis();
      IPAddress ntpIp;
      Serial.printf("Waiting for NTP... DNS=%s, resolve %s: %s\n", WiFi.dnsIP().toString().c_str(),
                    NTP_SERVER_1, WiFi.hostByName(NTP_SERVER_1, ntpIp) ? ntpIp.toString().c_str() : "FAILED");
    }
  }
  if (millis() - lastRead >= SENSOR_READ_INTERVAL_MS) {
    lastRead = millis();
    sampleSensor();
  }
}
