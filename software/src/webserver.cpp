#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>

#include <functional>

#include "config.h"
#include "filesystem.h"
#include "rtc.h"
#include "wifi.h"

#define BASE_PATH "/dashboard/"

static WebServer server(80);

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

static void handleNotFound() { server.send(404); }

static void send_json(uint16_t status, JsonDocument& response) {
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

static void handle_timezone_get(JsonDocument& request) {
    JsonDocument response;
    response["posix"] = config.timezone_posix;
    response["iana"] = config.timezone_iana;

    send_json(200, response);
}

static void handle_timezone_post(JsonDocument& request) {
    const char* timezone_posix = request["posix"];
    const char* timezone_iana = request["iana"];

    if (!timezone_posix || !timezone_iana) {
        server.send(400);
        return;
    }

    if (!rtc_set_timezone(timezone_posix)) {
        server.send(500);
        return;
    }

    config.timezone_posix = timezone_posix;
    config.timezone_iana = timezone_iana;
    if (!config_save()) {
        server.send(500);
        return;
    }

    JsonDocument response;
    response["posix"] = timezone_posix;
    response["iana"] = timezone_iana;

    send_json(200, response);
}

static void handle_time_display_format_get(JsonDocument& request) {
    JsonDocument response;
    response["format"] = config.time_display_format;

    send_json(200, response);
}

static void handle_time_display_format_post(JsonDocument& request) {
    uint8_t time_display_format = request["format"];

    if (!time_display_format ||
        (time_display_format != 12 && time_display_format != 24)) {
        server.send(400);
        return;
    }

    config.time_display_format = time_display_format;
    if (!config_save()) {
        server.send(500);
        return;
    }

    JsonDocument response;
    response["format"] = time_display_format;

    send_json(200, response);
}

// TODO: Button to reset config
static void setup_api() {
    on_api("/api/wifi", HTTP_POST, RequestBody::Json, handle_wifi_setup);
    on_api("/api/wifi", HTTP_GET, RequestBody::None, handle_wifi_status);
    on_api("/api/wifi", HTTP_DELETE, RequestBody::None, handle_wifi_forget);

    on_api("/api/time", HTTP_POST, RequestBody::Json, handle_time_set);

    on_api("/api/config/timezone", HTTP_GET, RequestBody::None,
           handle_timezone_get);
    on_api("/api/config/timezone", HTTP_POST, RequestBody::Json,
           handle_timezone_post);

    on_api("/api/config/time_display_format", HTTP_GET, RequestBody::None,
           handle_time_display_format_get);
    on_api("/api/config/time_display_format", HTTP_POST, RequestBody::Json,
           handle_time_display_format_post);
}

void webserver_setup() {
    server.addHandler(new AccessPointOnlyHandler());

    setup_api();

    if (filesystem_available) {
        server.serveStatic("/", LittleFS, BASE_PATH);
    }

    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("Web server listening on port 80");
}

void webserver_update() { server.handleClient(); }
