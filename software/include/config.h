#pragma once

#include <Arduino.h>

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
};

extern ClockConfig config;

void config_load();

bool config_save();
