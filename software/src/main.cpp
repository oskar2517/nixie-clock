#include "main.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <Wire.h>

#include "clock.h"
#include "clock_mdns.h"
#include "config.h"
#include "filesystem.h"
#include "pins.h"
#include "webserver.h"
#include "wifi.h"

void set_hv_enabled(bool enabled) { digitalWrite(PIN_SHDN, enabled); }

void write_version_to_file() {
    File file = LittleFS.open("/VERSION", FILE_WRITE);
    if (!file) return;

    file.print(FIRMWARE_VERSION);
    file.close();
}

void setup() {
    setup_pin(PIN_SHDN);

    Serial.begin(9600);
    Serial.println("Initializing clock...");

    Wire.begin(PIN_SDA, PIN_SCL);

    if (!filesystem_setup()) {
        Serial.println("LittleFS mount failed");
        return;
    }
    write_version_to_file();
    config_load();
    if (!clock_setup()) {
        Serial.println("Failed to initialize clock");
        return;
    }
    wifi_ap_setup();
    mdns_setup();
    webserver_setup();

    delay(1000);
    set_hv_enabled(true);
}

void loop() {
    clock_update();
    webserver_update();
}
