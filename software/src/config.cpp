#include "config.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

#define CONFIG_FILE "/config.json"

#define COPY2CONF(conf_name, doc_name) \
    config.conf_name = document[doc_name] | config.conf_name;
#define COPY2DOC(doc_name, conf_name) document[doc_name] = config.conf_name;

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

bool config_save() {
    Serial.println("Saving config file to LittleFS...");

    JsonDocument document;
    COPY2DOC("wifi_ssid", wifi_ssid)
    COPY2DOC("wifi_password", wifi_password)
    COPY2DOC("timezone_posix", timezone_posix)
    COPY2DOC("timezone_iana", timezone_iana)
    COPY2DOC("time_display_format", time_display_format)
    COPY2DOC("automatic_time", automatic_time)
    COPY2DOC("timer", timer)
    COPY2DOC("timer_tubes_off_hours", timer_tubes_off_hours)
    COPY2DOC("timer_tubes_off_minutes", timer_tubes_off_minutes)
    COPY2DOC("timer_tubes_on_hours", timer_tubes_on_hours)
    COPY2DOC("timer_tubes_on_minutes", timer_tubes_on_minutes)
    COPY2DOC("ntp_server", ntp_server);
    COPY2DOC("ntp_frequency", ntp_frequency)
    COPY2DOC("healing_mode", healing_mode)

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

    COPY2CONF(wifi_ssid, "wifi_ssid")
    COPY2CONF(wifi_password, "wifi_password")
    COPY2CONF(timezone_posix, "timezone_posix")
    COPY2CONF(timezone_iana, "timezone_iana")
    COPY2CONF(time_display_format, "time_display_format")
    COPY2CONF(automatic_time, "automatic_time")
    COPY2CONF(timer, "timer")
    COPY2CONF(timer_tubes_off_hours, "timer_tubes_off_hours")
    COPY2CONF(timer_tubes_off_minutes, "timer_tubes_off_minutes")
    COPY2CONF(timer_tubes_on_hours, "timer_tubes_on_hours")
    COPY2CONF(timer_tubes_on_minutes, "timer_tubes_on_minutes")
    COPY2CONF(ntp_server, "ntp_server")
    COPY2CONF(ntp_frequency, "ntp_frequency")
    COPY2CONF(healing_mode, "healing_mode")

    Serial.println("Loaded config file successfully");
}
