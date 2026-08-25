#include "LittleFS.h"

bool filesystem_available = false;

bool filesystem_setup() {
    filesystem_available = LittleFS.begin(false);

    if (!filesystem_available) {
        return false;
    }

    return true;
}
