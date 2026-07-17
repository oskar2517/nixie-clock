#include "config.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

#define CONFIG_FILE "/config.json"

JsonDocument config;

// TODO: Implement config validation
void create_default_config() {
    config.clear();

    config["timezone_posix"] = "CET-1CEST,M3.5.0/2,M10.5.0/3";
    config["timezone_iana"] = "Europe/Berlin";
    config["time_display_format"] = 24;

    Serial.println("Created default config");
}

bool config_save() {
    Serial.println("Saving config file to LittleFS...");

    String serialized;
    size_t json_size = serializeJson(config, serialized);
    if (json_size == 0) {
        Serial.println("Failed to serialize config");
        return false;
    }

    File file = LittleFS.open(CONFIG_FILE, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open config file for writing");
        return false;
    }

    size_t written = file.print(serialized);
    file.close();

    if (written != serialized.length()) {
        Serial.println("Failed to write complete config file");
        return false;
    }

    Serial.println("Saved config file successfully");

    return true;
}

void config_load() {
    Serial.println("Loading config file from LittleFS...");

    File file = LittleFS.open(CONFIG_FILE, FILE_READ);
    if (!file || file.isDirectory()) {
        Serial.println("Failed to open config file for reading");
        create_default_config();
        return;
    }

    DeserializationError error = deserializeJson(config, file);
    file.close();

    if (error) {
        Serial.print("Failed to parse config file: ");
        Serial.println(error.c_str());
        create_default_config();
        return;
    }

    Serial.println("Loaded config file successfully");
}
