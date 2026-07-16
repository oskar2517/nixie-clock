#include <LittleFS.h>
#include <WebServer.h>

#define BASE_PATH "/dashboard/"

static WebServer server(80);
static bool filesystem_available = false;

static void handleNotFound() { server.send(404, "text/plain", "not found"); }

void webserver_setup() {
    filesystem_available = LittleFS.begin(true);
    if (!filesystem_available) {
        Serial.println("LittleFS mount failed");
    }

    if (filesystem_available) {
        server.serveStatic("/", LittleFS, BASE_PATH);
    }

    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("Web server listening on port 80");
}

void webserver_update() { server.handleClient(); }
