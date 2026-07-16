#include "config.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

#define CONFIG_FILE "/config.json"

JsonDocument config;

void config_save() {
    Serial.println("Saving config file to LittleFS...");

    String serialized;
    size_t json_size = serializeJson(config, serialized);
    if (json_size == 0) {
        Serial.println("Failed to serialize config");
        return;
    }

    File file = LittleFS.open(CONFIG_FILE, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open config file for writing");
        return;
    }

    size_t written = file.print(serialized);
    file.close();

    if (written != serialized.length()) {
        Serial.println("Failed to write complete config file");
        return;
    }

    Serial.println("Saved config file successfully");
}

void config_load() {
    Serial.println("Loading config file from LittleFS...");

    File file = LittleFS.open(CONFIG_FILE, FILE_READ);
    if (!file || file.isDirectory()) {
        Serial.println("Failed to open config file for reading");
        config.clear();
        return;
    }

    DeserializationError error = deserializeJson(config, file);
    file.close();

    if (error) {
        Serial.print("Failed to parse config file: ");
        Serial.println(error.c_str());
        config.clear();
        return;
    }

    Serial.println("Loaded config file successfully");
}
