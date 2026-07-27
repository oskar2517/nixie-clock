#include "clock_mdns.h"

#include <Arduino.h>
#include <ESPmDNS.h>

void mdns_setup() {
    if (!MDNS.begin(MDNS_HOSTNAME)) {
        Serial.println("Failed to start mDNS");
        return;
    }

    MDNS.addService("http", "tcp", 80);

    Serial.print("mDNS started: http://");
    Serial.print(MDNS_HOSTNAME);
    Serial.println(".local");
}
