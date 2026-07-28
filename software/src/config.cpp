#include "config.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

#include "clock.h"
#include "rtc.h"

#define CONFIG_FILE "/config.json"

#define CONFIG_SECRET_FIELDS(FIELD) \
    FIELD(wifi_ssid, "wifiSsid")    \
    FIELD(wifi_password, "wifiPassword")

#define CONFIG_PUBLIC_FIELDS(FIELD)                   \
    FIELD(timezone_posix, "timezonePosix")            \
    FIELD(timezone_iana, "timezoneIana")              \
    FIELD(time_display_format, "timeDisplayFormat")   \
    FIELD(automatic_time, "automaticTime")            \
    FIELD(timer, "timer")                             \
    FIELD(timer_tubes_off_hours, "tubesOffHours")     \
    FIELD(timer_tubes_off_minutes, "tubesOffMinutes") \
    FIELD(timer_tubes_on_hours, "tubesOnHours")       \
    FIELD(timer_tubes_on_minutes, "tubesOnMinutes")   \
    FIELD(ntp_server, "ntpServer")                    \
    FIELD(ntp_frequency, "ntpFrequency")              \
    FIELD(healing_mode, "healingMode")                \
    FIELD(neons_mode, "neonsMode")                    \
    FIELD(acp_routine, "acpRoutine")                  \
    FIELD(neons_frequency, "neonsFrequency")          \
    FIELD(neons_brightness, "neonsBrightness")        \
    FIELD(digit_cross_fade, "digitCrossFade")

#define COPY2CONF(conf_name, doc_name) \
    target.conf_name = document[doc_name] | target.conf_name;

#define COPY2DOC(conf_name, doc_name) document[doc_name] = source.conf_name;

ClockConfig config;

static bool config_apply_side_effects(const ClockConfig& next);

// TODO: Implement config validation
static ClockConfig default_config() {
    ClockConfig next;

    next.wifi_ssid = "";
    next.wifi_password = "";
    next.timezone_posix = "CET-1CEST,M3.5.0/2,M10.5.0/3";
    next.timezone_iana = "Europe/Berlin";
    next.time_display_format = 24;
    next.automatic_time = true;
    next.timer = false;
    next.timer_tubes_off_hours = 22;
    next.timer_tubes_off_minutes = 0;
    next.timer_tubes_on_hours = 9;
    next.timer_tubes_on_minutes = 0;
    next.ntp_server = "pool.ntp.org";
    next.ntp_frequency = 60;
    next.healing_mode = false;
    next.neons_mode = CFG_NEONS_MODE_BLINK;
    next.acp_routine = 1;
    next.neons_frequency = 1000;
    next.neons_brightness = 70;
    next.digit_cross_fade = true;

    return next;
}

static void load_default_config() {
    config = default_config();
    Serial.println("Created default config");
}

static void config_to_json(const ClockConfig& source, JsonDocument& document,
                           bool include_secrets) {
    if (include_secrets) {
        CONFIG_SECRET_FIELDS(COPY2DOC)
    }

    CONFIG_PUBLIC_FIELDS(COPY2DOC)
}

void config_to_json(JsonDocument& document) {
    config_to_json(config, document, false);
}

void config_apply_json(ClockConfig& target, JsonDocument& document,
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

void config_load() {
    Serial.println("Loading config file from LittleFS...");

    File file = LittleFS.open(CONFIG_FILE, FILE_READ);
    if (!file || file.isDirectory()) {
        Serial.println("Failed to open config file for reading");
        load_default_config();
        return;
    }

    JsonDocument document;
    DeserializationError error = deserializeJson(document, file);
    file.close();

    if (error) {
        Serial.print("Failed to parse config file: ");
        Serial.println(error.c_str());
        load_default_config();
        return;
    }

    ClockConfig next = default_config();
    config_apply_json(next, document, true);
    config = next;

    Serial.println("Loaded config file successfully");
}

bool config_apply(const ClockConfig& next) {
    if (!config_apply_side_effects(next)) {
        return false;
    }

    config = next;
    return true;
}

bool config_reset_to_default() {
    if (!config_apply(default_config())) {
        return false;
    }

    Serial.println("Reset config to defaults");

    return true;
}

bool config_apply_side_effects(const ClockConfig& next) {
    if (next.timezone_posix != config.timezone_posix) {
        if (!rtc_set_timezone(next.timezone_posix.c_str())) {
            return false;
        }
    }

    if (next.neons_frequency != config.neons_frequency ||
        next.neons_brightness != config.neons_brightness) {
        clock_apply_neon_pwm_config(next.neons_frequency,
                                    next.neons_brightness);
    }

    return true;
}
