#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

enum NeonsMode : uint8_t {
    OFF,
    BLINK,
    TOGGLE,
};

struct ClockConfig {
    String wifi_ssid;
    String wifi_password;
    String wifi_ap_password;
    wifi_power_t wifi_idle_transmission_power;
    wifi_power_t wifi_connected_transmission_power;
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
    NeonsMode neons_mode;
    uint32_t neons_frequency;
    uint16_t neons_brightness;
    int8_t acp_routine;
    bool digit_cross_fade;
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
