#pragma once

#include <ArduinoJson.h>

extern JsonDocument config;

void config_load();

void config_save();
