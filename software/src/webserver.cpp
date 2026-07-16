#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WebServer.h>

#include <functional>

#include "wifi.h"

#define BASE_PATH "/dashboard/"

static WebServer server(80);
static bool filesystem_available = false;

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

static void handle_wifi_connect(JsonDocument& request) {
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

    JsonDocument response;
    response["ssid"] = ssid;
    send_json(200, response);
}

static void setup_api() {
    on_api("/api/wifi", HTTP_POST, RequestBody::Json, handle_wifi_connect);
}

void webserver_setup() {
    filesystem_available = LittleFS.begin(true);
    if (!filesystem_available) {
        Serial.println("LittleFS mount failed");
    }

    if (filesystem_available) {
        server.serveStatic("/", LittleFS, BASE_PATH);
    }

    setup_api();

    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("Web server listening on port 80");
}

void webserver_update() { server.handleClient(); }
