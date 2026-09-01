#include "config.h"

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <ctype.h>

#include "acp.h"
#include "clock.h"
#include "rtc.h"

#define CONFIG_FILE "/config.json"

#define WIFI_SSID_MAX_LENGTH 32
#define WIFI_PASSWORD_MAX_LENGTH 64
#define TIMEZONE_POSIX_MAX_LENGTH 96
#define TIMEZONE_IANA_MAX_LENGTH 64
#define NTP_SERVER_MAX_LENGTH 253
#define NTP_FREQUENCY_MIN_MINUTES 1
#define NTP_FREQUENCY_MAX_MINUTES 10080
#define NEONS_FREQUENCY_MIN_HZ 1
#define NEONS_FREQUENCY_MAX_HZ 40000

#define CONFIG_SECRET_FIELDS(FIELD)      \
    FIELD(wifi_ssid, "wifiSsid")         \
    FIELD(wifi_password, "wifiPassword") \
    FIELD(wifi_ap_password, "wifiApPassword")

#define CONFIG_PUBLIC_FIELDS(FIELD)                          \
    FIELD(timezone_posix, "timezonePosix")                   \
    FIELD(timezone_iana, "timezoneIana")                     \
    FIELD(time_display_format, "timeDisplayFormat")          \
    FIELD(automatic_time, "automaticTime")                   \
    FIELD(timer, "timer")                                    \
    FIELD(timer_tubes_off_hours, "tubesOffHours")            \
    FIELD(timer_tubes_off_minutes, "tubesOffMinutes")        \
    FIELD(timer_tubes_on_hours, "tubesOnHours")              \
    FIELD(timer_tubes_on_minutes, "tubesOnMinutes")          \
    FIELD(ntp_server, "ntpServer")                           \
    FIELD(ntp_frequency, "ntpFrequency")                     \
    FIELD(healing_mode, "healingMode")                       \
    FIELD(neons_mode, "neonsMode")                           \
    FIELD(acp_routine, "acpRoutine")                         \
    FIELD(neons_frequency, "neonsFrequency")                 \
    FIELD(neons_brightness, "neonsBrightness")               \
    FIELD(digit_cross_fade, "digitCrossFade")                \
    FIELD(wifi_idle_transmission_power, "wifiIdleTransmissionPower") \
    FIELD(wifi_connected_transmission_power, "wifiConnectedTransmissionPower")

#define COPY2DOC(conf_name, doc_name) document[doc_name] = source.conf_name;

ClockConfig config;

static bool config_apply_side_effects(const ClockConfig& next);

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
    next.neons_brightness = 60;
    next.digit_cross_fade = false;
    next.wifi_ap_password = "";
    next.wifi_connected_transmission_power = WIFI_POWER_19_5dBm;
    next.wifi_idle_transmission_power = WIFI_POWER_2dBm;

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

template <typename T>
static bool copy_json_field(JsonDocument& document, const char* name,
                            T& target) {
    JsonObjectConst object = document.as<JsonObjectConst>();
    JsonVariantConst value = object[name];
    if (value.isUnbound()) {
        return true;
    }

    if (value.isNull() || !value.is<T>()) {
        return false;
    }

    target = value.as<T>();
    return true;
}

void config_to_json(JsonDocument& document) {
    config_to_json(config, document, false);
}

bool config_apply_json(ClockConfig& target, JsonDocument& document,
                       bool include_secrets) {
    if (!document.is<JsonObject>()) {
        return false;
    }

    if (include_secrets) {
        if (!copy_json_field(document, "wifiSsid", target.wifi_ssid) ||
            !copy_json_field(document, "wifiPassword", target.wifi_password) ||
            !copy_json_field(document, "wifiApPassword",
                             target.wifi_ap_password)) {
            return false;
        }
    }

    return copy_json_field(document, "timezonePosix", target.timezone_posix) &&
           copy_json_field(document, "timezoneIana", target.timezone_iana) &&
           copy_json_field(document, "timeDisplayFormat",
                           target.time_display_format) &&
           copy_json_field(document, "automaticTime", target.automatic_time) &&
           copy_json_field(document, "timer", target.timer) &&
           copy_json_field(document, "tubesOffHours",
                           target.timer_tubes_off_hours) &&
           copy_json_field(document, "tubesOffMinutes",
                           target.timer_tubes_off_minutes) &&
           copy_json_field(document, "tubesOnHours",
                           target.timer_tubes_on_hours) &&
           copy_json_field(document, "tubesOnMinutes",
                           target.timer_tubes_on_minutes) &&
           copy_json_field(document, "ntpServer", target.ntp_server) &&
           copy_json_field(document, "ntpFrequency", target.ntp_frequency) &&
           copy_json_field(document, "healingMode", target.healing_mode) &&
           copy_json_field(document, "neonsMode", target.neons_mode) &&
           copy_json_field(document, "acpRoutine", target.acp_routine) &&
           copy_json_field(document, "neonsFrequency",
                           target.neons_frequency) &&
           copy_json_field(document, "neonsBrightness",
                           target.neons_brightness) &&
           copy_json_field(document, "digitCrossFade",
                           target.digit_cross_fade) &&
           copy_json_field(document, "wifiIdleTransmissionPower",
                           target.wifi_idle_transmission_power) &&
           copy_json_field(document, "wifiConnectedTransmissionPower",
                           target.wifi_connected_transmission_power);
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
        config_save();
        return;
    }

    JsonDocument document;
    DeserializationError error = deserializeJson(document, file);
    file.close();

    if (error) {
        Serial.print("Failed to parse config file: ");
        Serial.println(error.c_str());
        load_default_config();
        config_save();
        return;
    }

    ClockConfig next = default_config();
    if (!config_apply_json(next, document, true) || !config_validate(next)) {
        Serial.println("Config file contains invalid values");
        load_default_config();
        config_save();
        return;
    }

    config = next;

    Serial.println("Loaded config file successfully");
}

bool config_apply(const ClockConfig& next) {
    if (!config_validate(next)) {
        return false;
    }

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

static bool text_field_is_valid(const String& value, size_t max_length,
                                bool allow_empty) {
    if (!allow_empty && value.length() == 0) {
        return false;
    }

    if (value.length() > max_length) {
        return false;
    }

    for (size_t i = 0; i < value.length(); i++) {
        uint8_t c = static_cast<uint8_t>(value[i]);
        if (c < 32 || c == 127) {
            return false;
        }
    }

    return true;
}

static bool host_field_is_valid(const String& value) {
    if (!text_field_is_valid(value, NTP_SERVER_MAX_LENGTH, false)) {
        return false;
    }

    for (size_t i = 0; i < value.length(); i++) {
        if (isspace(static_cast<unsigned char>(value[i]))) {
            return false;
        }
    }

    return true;
}

static bool wifi_transmission_power_is_valid(wifi_power_t power) {
    return power == WIFI_POWER_19_5dBm || power == WIFI_POWER_19dBm ||
           power == WIFI_POWER_18_5dBm || power == WIFI_POWER_17dBm ||
           power == WIFI_POWER_15dBm || power == WIFI_POWER_13dBm ||
           power == WIFI_POWER_11dBm || power == WIFI_POWER_8_5dBm ||
           power == WIFI_POWER_7dBm || power == WIFI_POWER_5dBm ||
           power == WIFI_POWER_2dBm || power == WIFI_POWER_MINUS_1dBm;
}

bool config_validate(const ClockConfig& candidate) {
    if (!text_field_is_valid(candidate.wifi_ssid, WIFI_SSID_MAX_LENGTH, true) ||
        !text_field_is_valid(candidate.wifi_password, WIFI_PASSWORD_MAX_LENGTH,
                             true) ||
        !text_field_is_valid(candidate.wifi_ap_password,
                             WIFI_PASSWORD_MAX_LENGTH, true) ||
        !text_field_is_valid(candidate.timezone_posix,
                             TIMEZONE_POSIX_MAX_LENGTH, false) ||
        !text_field_is_valid(candidate.timezone_iana, TIMEZONE_IANA_MAX_LENGTH,
                             false) ||
        !host_field_is_valid(candidate.ntp_server)) {
        return false;
    }

    if (candidate.wifi_ap_password.length() > 0 &&
        candidate.wifi_ap_password.length() < 8) {
        return false;
    }

    if (!wifi_transmission_power_is_valid(candidate.wifi_idle_transmission_power)) {
        return false;
    }

    if (!wifi_transmission_power_is_valid(
            candidate.wifi_connected_transmission_power)) {
        return false;
    }

    if (candidate.time_display_format != 12 &&
        candidate.time_display_format != 24) {
        return false;
    }

    if (candidate.timer_tubes_off_hours > 23 ||
        candidate.timer_tubes_on_hours > 23 ||
        candidate.timer_tubes_off_minutes > 59 ||
        candidate.timer_tubes_on_minutes > 59) {
        return false;
    }

    if (candidate.ntp_frequency < NTP_FREQUENCY_MIN_MINUTES ||
        candidate.ntp_frequency > NTP_FREQUENCY_MAX_MINUTES) {
        return false;
    }

    if (candidate.neons_mode > CFG_NEONS_MODE_TOGGLE) {
        return false;
    }

    if (candidate.acp_routine < -1 ||
        candidate.acp_routine >= static_cast<int8_t>(acp_routine_count)) {
        return false;
    }

    if (candidate.neons_frequency < NEONS_FREQUENCY_MIN_HZ ||
        candidate.neons_frequency > NEONS_FREQUENCY_MAX_HZ ||
        candidate.neons_brightness > 100) {
        return false;
    }

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

    bool wifi_transmission_power_changed =
        next.wifi_idle_transmission_power != config.wifi_idle_transmission_power ||
        next.wifi_connected_transmission_power !=
            config.wifi_connected_transmission_power;
    if (wifi_transmission_power_changed && WiFi.status() != WL_CONNECTED) {
        WiFi.setTxPower(next.wifi_idle_transmission_power);
    }

    return true;
}
