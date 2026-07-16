#pragma once

#define WIFI_AP_SSID "Nixie Clock"
#define WIFI_AP_PASSWORD "123456pass"

void wifi_ap_setup();

bool wifi_connect(const char* ssid, const char* password);

void wifi_disconnect();
