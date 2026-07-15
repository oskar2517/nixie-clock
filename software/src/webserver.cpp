#include "webserver.h"

#include <WebServer.h>

static WebServer server(80);

static void handleRoot() {
    server.send(200, "text/plain", "hello from esp32!");
}

static void handleNotFound() {
    server.send(404, "text/plain", "not found");
}

void webserver_setup() {
    server.on("/", handleRoot);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("Web server listening on port 80");
}

void webserver_update() {
    server.handleClient();
}
