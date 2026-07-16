#pragma once

#define WIFI_AP_SSID "Nixie Clock"
#define WIFI_AP_PASSWORD "123456pass"

void wifi_ap_setup();

bool wifi_connect(const char* ssid, const char* password);

bool wifi_connect_with_config_credentials();

void wifi_disconnect();

