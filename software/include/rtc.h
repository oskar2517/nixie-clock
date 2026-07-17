#pragma once

#include <time.h>

bool rtc_ntp_fetch_time();

bool rtc_set_unix_time(time_t unix_timestamp);

bool rtc_set_timezone(const char* posix_timezone);
