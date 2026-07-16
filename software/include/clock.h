#pragma once

#include "RTClib.h"

extern RTC_DS3231 rtc;

void clock_setup();

void clock_update();