#include "config.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

#define CONFIG_FILE "/config.json"

#define CONFIG_SECRET_FIELDS(FIELD) \
    FIELD(wifi_ssid, "wifiSsid")    \
    FIELD(wifi_password, "wifiPassword")

#define CONFIG_PUBLIC_FIELDS(FIELD)                              \
    FIELD(timezone_posix, "timezonePosix")                       \
    FIELD(timezone_iana, "timezoneIana")                         \
    FIELD(time_display_format, "timeDisplayFormat")              \
    FIELD(automatic_time, "automaticTime")                       \
    FIELD(timer, "timer")                                        \
    FIELD(timer_tubes_off_hours, "tubesOffHours")                \
    FIELD(timer_tubes_off_minutes, "tubesOffMinutes")            \
    FIELD(timer_tubes_on_hours, "tubesOnHours")                  \
    FIELD(timer_tubes_on_minutes, "tubesOnMinutes")              \
    FIELD(ntp_server, "ntpServer")                               \
    FIELD(ntp_frequency, "ntpFrequency")                         \
    FIELD(healing_mode, "healingMode")

#define COPY2CONF(conf_name, doc_name) target.conf_name = document[doc_name] | target.conf_name;

#define COPY2DOC(conf_name, doc_name) document[doc_name] = source.conf_name;

ClockConfig config;

// TODO: Implement config validation
static void set_default_config() {
    config.wifi_ssid = "";
    config.wifi_password = "";
    config.timezone_posix = "CET-1CEST,M3.5.0/2,M10.5.0/3";
    config.timezone_iana = "Europe/Berlin";
    config.time_display_format = 24;
    config.automatic_time = true;
    config.timer = false;
    config.timer_tubes_off_hours = 22;
    config.timer_tubes_off_minutes = 0;
    config.timer_tubes_on_hours = 9;
    config.timer_tubes_on_minutes = 0;
    config.ntp_server = "pool.ntp.org";
    config.ntp_frequency = 60;
    config.healing_mode = false;
}

static void create_default_config() {
    set_default_config();
    Serial.println("Created default config");
}

static void config_to_json(const ClockConfig& source, JsonDocument& document,
                           bool include_secrets) {
    if (include_secrets) {
        CONFIG_SECRET_FIELDS(COPY2DOC)
    }

    CONFIG_PUBLIC_FIELDS(COPY2DOC)
}

static void config_apply_json(ClockConfig& target, JsonDocument& document,
                              bool include_secrets) {
    if (include_secrets) {
        CONFIG_SECRET_FIELDS(COPY2CONF)
    }

    CONFIG_PUBLIC_FIELDS(COPY2CONF)
}

bool config_save() {
    Serial.println("Saving config file to LittleFS...");

    JsonDocument document;
    config_to_json(config, document, true);

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

void config_to_json(JsonDocument& document) {
    config_to_json(config, document, false);
}

void config_apply_json(ClockConfig& target, JsonDocument& document) {
    config_apply_json(target, document, false);
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
    config_apply_json(config, document, true);

    Serial.println("Loaded config file successfully");
}
