#include <Arduino.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>

#include <functional>

#include "acp.h"
#include "clock.h"
#include "config.h"
#include "filesystem.h"
#include "main.h"
#include "rtc.h"
#include "wifi.h"

#define BASE_PATH "/dashboard/"
#define DNS_PORT 53

static WebServer server(80);
static DNSServer dns_server;

static bool request_from_access_point() {
    return server.client().localIP() == WiFi.softAPIP();
}

class AccessPointOnlyHandler : public RequestHandler {
   public:
    bool canHandle(HTTPMethod requestMethod, String requestUri) override {
        (void)requestMethod;
        (void)requestUri;

        return !request_from_access_point();
    }

    bool handle(WebServer& server, HTTPMethod requestMethod,
                String requestUri) override {
        (void)requestMethod;
        (void)requestUri;

        server.send(403);
        return true;
    }
};

enum class RequestBody : uint8_t {
    None,
    Json,
};

using ApiHandler = std::function<void(JsonDocument& request)>;

static String dashboard_url() {
    return String("http://") + WiFi.softAPIP().toString() + "/";
}

static void redirect_to_dashboard() {
    server.sendHeader("Location", dashboard_url(), true);
    server.send(302, "text/plain", "");
}

static void handleNotFound() {
    if (server.uri().startsWith("/api/")) {
        server.send(404);
        return;
    }

    if (filesystem_available) {
        redirect_to_dashboard();
        return;
    }

    server.send(200, "text/plain", "Nixie Clock dashboard unavailable");
}

static const char* wifi_mode_name(wifi_mode_t mode) {
    switch (mode) {
        case WIFI_OFF:
            return "off";
        case WIFI_STA:
            return "sta";
        case WIFI_AP:
            return "ap";
        case WIFI_AP_STA:
            return "ap_sta";
        default:
            return "unknown";
    }
}

static String datetime_to_iso(const DateTime& datetime) {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02d",
             datetime.year(), datetime.month(), datetime.day(),
             datetime.hour(), datetime.minute(), datetime.second());

    return String(buffer);
}

template <typename T>
static void send_json(uint16_t status, const T& response) {
    String serialized;
    serializeJson(response, serialized);

    server.send(status, "application/json", serialized);
}

static void on_api(const char* uri, HTTPMethod method, RequestBody body,
                   ApiHandler handler) {
    server.on(uri, method, [body, handler]() {
        JsonDocument request;

        if (body == RequestBody::Json) {
            DeserializationError error =
                deserializeJson(request, server.arg("plain"));

            if (error) {
                server.send(400);
                return;
            }
        }

        handler(request);
    });
}

static void handle_wifi_setup(JsonDocument& request) {
    const char* ssid = request["ssid"];
    const char* password = request["password"];

    if (!ssid || !password) {
        server.send(400);
        return;
    }

    if (!wifi_connect(ssid, password)) {
        server.send(403);
        return;
    }

    wifi_disconnect();

    config.wifi_ssid = ssid;
    config.wifi_password = password;
    if (!config_save()) {
        server.send(500);
        return;
    }

    JsonDocument response;
    response["ssid"] = ssid;
    send_json(200, response);
}

static void handle_wifi_status(JsonDocument& request) {
    if (config.wifi_ssid.length() == 0) {
        server.send(404);
        return;
    }

    JsonDocument response;
    response["ssid"] = config.wifi_ssid;
    send_json(200, response);
}

static void handle_wifi_forget(JsonDocument& request) {
    config.wifi_ssid = "";
    config.wifi_password = "";

    if (!config_save()) {
        server.send(500);
        return;
    }

    server.send(204);
}

static void handle_wifi_wifi_scan(JsonDocument& request) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect(false);
    int16_t n = WiFi.scanNetworks();

    JsonDocument response;
    JsonArray networks = response.to<JsonArray>();

    if (n < 0) {
        WiFi.scanDelete();
        WiFi.mode(WIFI_AP);
        server.send(500);
        return;
    }

    for (int16_t i = 0; i < n; i++) {
        JsonObject network = networks.add<JsonObject>();

        network["ssid"] = WiFi.SSID(i);
        network["rssi"] = WiFi.RSSI(i);
        network["channel"] = WiFi.channel(i);
        network["open"] = WiFi.encryptionType(i) == WIFI_AUTH_OPEN;
    }

    WiFi.scanDelete();
    WiFi.mode(WIFI_AP);

    send_json(200, response);
}

static void handle_time_set(JsonDocument& request) {
    int64_t unix_timestamp = request["timestamp"] | 0;

    if (unix_timestamp <= 0 || unix_timestamp > INT32_MAX) {
        server.send(400);
        return;
    }

    if (rtc_set_unix_time((time_t)unix_timestamp)) {
        server.send(200);
    } else {
        server.send(500);
    }
}

static void handle_config_get(JsonDocument& request) {
    JsonDocument response;
    config_to_json(response);

    send_json(200, response);
}

static bool apply_config_side_effects(const ClockConfig& next) {
    if (next.timezone_posix != config.timezone_posix) {
        if (!rtc_set_timezone(next.timezone_posix.c_str())) {
            return false;
        }
    }

    if (next.neons_frequency != config.neons_frequency) {
        clock_set_neon_pwm_frequency(config.neons_frequency);
    }

    return true;
}

static void handle_config_post(JsonDocument& request) {
    ClockConfig next = config;
    config_apply_json(next, request);

    if (!apply_config_side_effects(next)) {
        server.send(500);
        return;
    }

    config = next;
    if (!config_save()) {
        server.send(500);
        return;
    }

    JsonDocument response;
    config_to_json(response);

    send_json(200, response);
}

static void handle_config_delete(JsonDocument& request) {
    config_create_default();
    if (!config_save()) {
        server.send(500);
        return;
    }

    JsonDocument response;
    config_to_json(response);

    send_json(200, response);
}

static void handle_acp_test_post(JsonDocument& request) {
    clock_start_acp_routine();

    server.send(204);
}

static void handle_firmware_get(JsonDocument& request) {
    JsonDocument response;
    response["version"] = FIRMWARE_VERSION;

    send_json(200, response);
}

static void handle_diagnostics_get(JsonDocument& request) {
    JsonDocument response;

    JsonObject firmware = response["firmware"].to<JsonObject>();
    firmware["version"] = FIRMWARE_VERSION;

    JsonObject system = response["system"].to<JsonObject>();
    system["uptimeMs"] = millis();
    system["chipModel"] = ESP.getChipModel();
    system["chipRevision"] = ESP.getChipRevision();
    system["cpuMhz"] = ESP.getCpuFreqMHz();
    system["sdkVersion"] = ESP.getSdkVersion();
    system["freeHeap"] = ESP.getFreeHeap();
    system["minFreeHeap"] = ESP.getMinFreeHeap();
    system["sketchSize"] = ESP.getSketchSize();
    system["freeSketchSpace"] = ESP.getFreeSketchSpace();

    JsonObject wifi = response["wifi"].to<JsonObject>();
    wifi["mode"] = wifi_mode_name(WiFi.getMode());
    wifi["apSsid"] = WIFI_AP_SSID;
    wifi["apIp"] = WiFi.softAPIP().toString();
    wifi["apConnectedClients"] = WiFi.softAPgetStationNum();

    wl_status_t station_status = WiFi.status();
    wifi["stationConnected"] = station_status == WL_CONNECTED;
    wifi["stationStatus"] = station_status;
    wifi["stationSsid"] = WiFi.SSID();
    wifi["stationIp"] = WiFi.localIP().toString();
    if (station_status == WL_CONNECTED) {
        wifi["stationRssi"] = WiFi.RSSI();
    }

    JsonObject filesystem = response["filesystem"].to<JsonObject>();
    filesystem["available"] = filesystem_available;
    if (filesystem_available) {
        size_t total_bytes = LittleFS.totalBytes();
        size_t used_bytes = LittleFS.usedBytes();

        filesystem["totalBytes"] = total_bytes;
        filesystem["usedBytes"] = used_bytes;
        filesystem["freeBytes"] = total_bytes - used_bytes;
    }

    JsonObject rtc_diagnostics = response["rtc"].to<JsonObject>();
    rtc_diagnostics["available"] = clock_rtc_available();
    if (clock_rtc_available()) {
        DateTime now = rtc.now();
        rtc_diagnostics["lostPower"] = rtc.lostPower();
        rtc_diagnostics["unixTime"] = now.unixtime();
        rtc_diagnostics["dateTime"] = datetime_to_iso(now);
    }

    JsonObject clock = response["clock"].to<JsonObject>();
    clock["digitCount"] = clock_digit_count;
    clock["acpRunning"] = clock_acp_routine_running();
    clock["acpRoutineCount"] = acp_routine_count;

    send_json(200, response);
}

static void setup_api() {
    on_api("/api/wifi", HTTP_POST, RequestBody::Json, handle_wifi_setup);
    on_api("/api/wifi", HTTP_GET, RequestBody::None, handle_wifi_status);
    on_api("/api/wifi", HTTP_DELETE, RequestBody::None, handle_wifi_forget);
    on_api("/api/wifi/networks", HTTP_GET, RequestBody::None,
           handle_wifi_wifi_scan);

    on_api("/api/time", HTTP_POST, RequestBody::Json, handle_time_set);

    on_api("/api/config", HTTP_GET, RequestBody::None, handle_config_get);
    on_api("/api/config", HTTP_POST, RequestBody::Json, handle_config_post);
    on_api("/api/config", HTTP_DELETE, RequestBody::None, handle_config_delete);

    on_api("/api/acp_test", HTTP_POST, RequestBody::None, handle_acp_test_post);

    on_api("/api/firmware", HTTP_GET, RequestBody::None, handle_firmware_get);
    on_api("/api/diagnostics", HTTP_GET, RequestBody::None,
           handle_diagnostics_get);
}

static void setup_captive_portal() {
    IPAddress ap_ip = WiFi.softAPIP();

    dns_server.start(DNS_PORT, "*", ap_ip);

    server.on("/generate_204", HTTP_GET, redirect_to_dashboard);
    server.on("/gen_204", HTTP_GET, redirect_to_dashboard);
    server.on("/hotspot-detect.html", HTTP_GET, redirect_to_dashboard);
    server.on("/library/test/success.html", HTTP_GET, redirect_to_dashboard);
    server.on("/ncsi.txt", HTTP_GET, redirect_to_dashboard);
    server.on("/connecttest.txt", HTTP_GET, redirect_to_dashboard);
    server.on("/redirect", HTTP_GET, redirect_to_dashboard);
    server.on("/canonical.html", HTTP_GET, redirect_to_dashboard);
    server.on("/success.txt", HTTP_GET, redirect_to_dashboard);

    Serial.print("Captive portal DNS listening at ");
    Serial.println(ap_ip);
}

void webserver_setup() {
    server.addHandler(new AccessPointOnlyHandler());

    setup_api();
    setup_captive_portal();

    if (filesystem_available) {
        server.serveStatic("/", LittleFS, BASE_PATH);
    }

    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("Web server listening on port 80");
}

void webserver_update() {
    dns_server.processNextRequest();
    server.handleClient();
}
