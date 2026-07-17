#include "config.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

#define CONFIG_FILE "/config.json"

ClockConfig config;

// TODO: Implement config validation
static void set_default_config() {
    config.wifi_ssid = "";
    config.wifi_password = "";
    config.timezone_posix = "CET-1CEST,M3.5.0/2,M10.5.0/3";
    config.timezone_iana = "Europe/Berlin";
    config.time_display_format = 24;
}

static void create_default_config() {
    set_default_config();
    Serial.println("Created default config");
}

bool config_save() {
    Serial.println("Saving config file to LittleFS...");

    JsonDocument document;
    document["timezone_posix"] = config.timezone_posix;
    document["timezone_iana"] = config.timezone_iana;
    document["time_display_format"] = config.time_display_format;

    if (config.wifi_ssid.length() > 0) {
        document["wifi_ssid"] = config.wifi_ssid;
    }

    if (config.wifi_password.length() > 0) {
        document["wifi_password"] = config.wifi_password;
    }

    String serialized;
    size_t json_size = serializeJson(document, serialized);
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

    JsonDocument document;
    DeserializationError error = deserializeJson(document, file);
    file.close();

    if (error) {
        Serial.print("Failed to parse config file: ");
        Serial.println(error.c_str());
        create_default_config();
        return;
    }

    set_default_config();

    config.wifi_ssid = document["wifi_ssid"] | "";
    config.wifi_password = document["wifi_password"] | "";
    config.timezone_posix =
        document["timezone_posix"] | config.timezone_posix.c_str();
    config.timezone_iana =
        document["timezone_iana"] | config.timezone_iana.c_str();
    config.time_display_format =
        document["time_display_format"] | config.time_display_format;

    Serial.println("Loaded config file successfully");
}
