#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WebServer.h>

#include <functional>

#include "config.h"
#include "filesystem.h"
#include "wifi.h"

#define BASE_PATH "/dashboard/"

static WebServer server(80);

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

    config["wifi_ssid"] = ssid;
    config["wifi_password"] = password;
    config_save();

    JsonDocument response;
    response["ssid"] = ssid;
    send_json(200, response);
}

static void handle_wifi_status(JsonDocument& request) {
    const char* ssid = config["wifi_ssid"];

    if (!ssid) {
        server.send(404);
        return;
    }

    JsonDocument response;
    response["ssid"] = ssid;
    send_json(200, response);
}

static void handle_wifi_forget(JsonDocument& request) {
    config.remove("wifi_ssid");
    config.remove("wifi_password");

    config_save();

    server.send(204);
}

static void setup_api() {
    on_api("/api/wifi", HTTP_POST, RequestBody::Json, handle_wifi_setup);
    on_api("/api/wifi", HTTP_GET, RequestBody::None, handle_wifi_status);
    on_api("/api/wifi", HTTP_DELETE, RequestBody::None, handle_wifi_forget);
}

void webserver_setup() {
    setup_api();

    if (filesystem_available) {
        server.serveStatic("/", LittleFS, BASE_PATH);
    }

    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("Web server listening on port 80");
}

void webserver_update() { server.handleClient(); }
