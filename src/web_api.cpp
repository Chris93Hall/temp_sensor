#include "web_api.h"
#include <ESP8266WebServer.h>
#include <time.h>

static ESP8266WebServer server;
static Reading latest;
static uint32_t rebootAt = 0;  // millis() deadline; 0 = no reboot pending

static String isoTime(time_t t) {
  char buf[25];
  struct tm tm;
  gmtime_r(&t, &tm);
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
  return String(buf);
}

static String readingJson() {
  String j = "{";
  if (latest.valid) {
    j += "\"temperature_c\":" + String(latest.temperatureC, 1);
    j += ",\"temperature_f\":" + String(latest.temperatureC * 9.0f / 5.0f + 32.0f, 1);
    j += ",\"humidity_percent\":" + String(latest.humidity, 1);
    if (latest.timeSynced) {
      j += ",\"timestamp\":\"" + isoTime(latest.unixTime) + "\"";
      j += ",\"timestamp_unix\":" + String((unsigned long)latest.unixTime);
    } else {
      j += ",\"timestamp\":null,\"timestamp_unix\":null";
    }
  } else {
    j += "\"error\":\"" + String(latest.lastError) + "\"";
  }
  j += "}";
  return j;
}

static void handleJson() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(latest.valid ? 200 : 503, "application/json", readingJson());
}

static void handleHtml() {
  String h = F("<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
               "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
               "<meta http-equiv=\"refresh\" content=\"10\">"
               "<title>Temperature &amp; Humidity</title><style>"
               "body{font-family:sans-serif;max-width:24em;margin:2em auto;padding:0 1em;color:#222}"
               "table{width:100%;border-collapse:collapse}"
               "td{padding:.6em;border-bottom:1px solid #ddd}td+td{text-align:right;font-weight:bold}"
               "small{color:#777}</style></head><body><h1>Temperature &amp; Humidity</h1>");
  if (latest.valid) {
    h += "<table><tr><td>Temperature</td><td>" + String(latest.temperatureC, 1) + " &deg;C / " +
         String(latest.temperatureC * 9.0f / 5.0f + 32.0f, 1) + " &deg;F</td></tr>";
    h += "<tr><td>Humidity</td><td>" + String(latest.humidity, 1) + " %</td></tr>";
    if (latest.timeSynced) {
      h += "<tr><td>Timestamp (ISO)</td><td>" + isoTime(latest.unixTime) + "</td></tr>";
      h += "<tr><td>Timestamp (Unix)</td><td>" + String((unsigned long)latest.unixTime) + "</td></tr>";
    } else {
      h += F("<tr><td>Timestamp</td><td>clock not synced</td></tr>");
    }
    h += "</table>";
  } else {
    h += "<p>No reading available: " + String(latest.lastError) + "</p>";
  }
  h += F("<p><small>Auto-refreshes every 10 s. JSON: <a href=\"/api/data\">/api/data</a>, "
         "<a href=\"/api/uptime\">/api/uptime</a></small></p></body></html>");
  server.send(200, "text/html", h);
}

static void handleUptime() {
  unsigned long secs = millis() / 1000;  // millis() wraps after ~49 days
  String j = "{\"uptime_seconds\":" + String(secs);
  j += ",\"uptime_human\":\"" + String(secs / 86400) + "d " + String((secs / 3600) % 24) + "h " +
       String((secs / 60) % 60) + "m " + String(secs % 60) + "s\"}";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", j);
}

static void handleReboot() {
  if (server.method() != HTTP_POST) {
    server.sendHeader("Allow", "POST");
    server.send(405, "application/json", "{\"error\":\"use POST\"}");
    return;
  }
  server.send(200, "application/json", "{\"status\":\"rebooting\"}");
  rebootAt = millis() + 500;  // let the response flush first
  if (rebootAt == 0) rebootAt = 1;
}

void webApiBegin(uint16_t port) {
  server.begin(port);
  server.on("/", handleHtml);
  server.on("/api/data", handleJson);
  server.on("/api/uptime", handleUptime);
  server.on("/api/reboot", handleReboot);
  server.onNotFound([]() { server.send(404, "application/json", "{\"error\":\"not found\"}"); });
}

void webApiHandle() {
  server.handleClient();
  if (rebootAt && (int32_t)(millis() - rebootAt) >= 0) ESP.restart();
}
void webApiUpdate(const Reading &r) { latest = r; }
