#pragma once

#include <time.h>

#include "RTClib.h"

extern RTC_DS3231 rtc;

bool rtc_is_available();

bool rtc_ntp_fetch_time();

bool rtc_set_unix_time(time_t unix_timestamp);

bool rtc_set_timezone(const char* posix_timezone);

bool rtc_init();
