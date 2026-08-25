#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#define CFG_NEONS_MODE_DISABLED 0
#define CFG_NEONS_MODE_BLINK 1
#define CFG_NEONS_MODE_TOGGLE 2

struct ClockConfig {
    String wifi_ssid;
    String wifi_password;
    String timezone_posix;
    String timezone_iana;
    uint8_t time_display_format;
    bool automatic_time;
    bool timer;
    uint8_t timer_tubes_off_hours;
    uint8_t timer_tubes_off_minutes;
    uint8_t timer_tubes_on_hours;
    uint8_t timer_tubes_on_minutes;
    String ntp_server;
    uint16_t ntp_frequency;
    bool healing_mode;
    uint8_t neons_mode;
    uint32_t neons_frequency;
    uint16_t neons_brightness;
    int8_t acp_routine;
    bool digit_cross_fade;
    String wifi_ap_password;
};

extern ClockConfig config;

void config_load();

bool config_save();

void config_to_json(JsonDocument& document);

bool config_apply_json(ClockConfig& target, JsonDocument& document,
                       bool include_secrets);

bool config_validate(const ClockConfig& candidate);

bool config_apply(const ClockConfig& next);

bool config_reset_to_default();
