#include "LittleFS.h"

bool filesystem_available = false;

void filesystem_setup() {
    filesystem_available = LittleFS.begin(false);

    if (!filesystem_available) {
        Serial.println("LittleFS mount failed");
    }
}
