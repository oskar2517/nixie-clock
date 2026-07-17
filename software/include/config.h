#pragma once

#include <ArduinoJson.h>

extern JsonDocument config;

void config_load();

bool config_save();
