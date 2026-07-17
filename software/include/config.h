#pragma once

#include <Arduino.h>

struct ClockConfig {
    String wifi_ssid;
    String wifi_password;
    String timezone_posix;
    String timezone_iana;
    uint8_t time_display_format;
    bool automatic_time;
};

extern ClockConfig config;

void config_load();

bool config_save();
