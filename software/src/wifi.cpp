#include "wifi.h"

#include <WiFi.h>

#include "config.h"

bool wifi_connect_with_config_credentials() {
    if (config.wifi_ssid.length() == 0 || config.wifi_password.length() == 0) {
        return false;
    }

    return wifi_connect(config.wifi_ssid.c_str(), config.wifi_password.c_str());
}

bool wifi_connect(const char* ssid, const char* password) {
    Serial.print("Connecting to WIFI with SSID ");
    Serial.print(ssid);

    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid, password);

    uint8_t attempt = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        attempt++;
        if (attempt > 10) {
            Serial.println("");
            Serial.println("Failed to connect to WIFI.");
            wifi_disconnect();
            return false;
        }
    }
    Serial.println("");

    Serial.println("WiFi connected.");

    return true;
}

void wifi_disconnect() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);
    Serial.println("WiFi disconnected.");
}

void wifi_ap_setup() {
    Serial.println("Setting access point...");

    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD)) {
        Serial.println("Failed to setup access point");
        return;
    }

    Serial.println("Sucessfully set up access point");

    IPAddress ip = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(ip);
}
