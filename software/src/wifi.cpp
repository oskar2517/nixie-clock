#include "wifi.h"

#include <WiFi.h>

void wifi_setup() {
    Serial.print("Setting Access Point...");

    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(WIFI_SSID, WIFI_PASSWORD)) {
        Serial.println(" failed");
        return;
    }

    Serial.println(" done");

    IPAddress ip = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(ip);
}
