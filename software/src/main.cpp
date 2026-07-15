#include <Arduino.h>
#include <Wire.h>

#include "clock.h"
#include "pins.h"
#include "webserver.h"
#include "wifi.h"

static void set_hv_enabled(boolean enabled) { digitalWrite(PIN_SHDN, enabled); }

void setup() {
    setup_pin(PIN_SHDN);

    Serial.begin(9600);
    Serial.println("Initializing clock...");

    Wire.begin(PIN_SDA, PIN_SCL);

    clock_setup();
    wifi_setup();
    webserver_setup();

    delay(1000);
    set_hv_enabled(true);
}

void loop() {
    clock_update();
    webserver_update();
}
