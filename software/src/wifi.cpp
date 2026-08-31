#include "wifi.h"

#include <WiFi.h>

#include "config.h"

String get_wifi_ap_password() {
    if (config.wifi_ap_password.length() > 0) return config.wifi_ap_password;

    return WIFI_AP_PASSWORD;
}

bool wifi_connect_with_config_credentials() {
    if (config.wifi_ssid.length() == 0) {
        return false;
    }

    return wifi_connect(config.wifi_ssid.c_str(), config.wifi_password.c_str());
}

bool wifi_connect(const char* ssid, const char* password) {
    Serial.print("Connecting to WIFI with SSID ");
    Serial.print(ssid);

    WiFi.mode(WIFI_AP_STA);
    WiFi.setTxPower(config.wifi_connected_transmission_power);
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
    WiFi.setTxPower(config.wifi_idle_transmission_power);
    Serial.println("WiFi disconnected.");
}

void wifi_ap_setup() {
    Serial.println("Setting access point...");

    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(WIFI_AP_SSID, get_wifi_ap_password())) {
        Serial.println("Failed to setup access point");
        return;
    }

    Serial.println("Sucessfully set up access point");

    IPAddress ip = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(ip);

    WiFi.setTxPower(config.wifi_idle_transmission_power);
}
