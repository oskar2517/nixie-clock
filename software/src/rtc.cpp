#include "RTClib.h"
#include "clock.h"
#include "wifi.h"

#include <sys/time.h>
#include <time.h>

#define NTP_SERVER "pool.ntp.org"
#define TIMEZONE "CET-1CEST,M3.5.0/2,M10.5.0/3"

static bool adjust_rtc() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to local obtain time.");
        return false;
    }

    int16_t year = timeinfo.tm_year + 1900;
    int16_t month = timeinfo.tm_mon + 1;
    int16_t day = timeinfo.tm_mday;
    int16_t hour = timeinfo.tm_hour;
    int16_t minute = timeinfo.tm_min;
    int16_t second = timeinfo.tm_sec;

    rtc.adjust(DateTime(year, month, day, hour, minute, second));
    Serial.print("Time adjusted to ");
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

    return true;
}

bool rtc_ntp_fetch_time() {
    if (!wifi_connect_with_config_credentials()) return false;

    Serial.print("Fetching time from NTP server ");
    Serial.println(NTP_SERVER);

    configTzTime(TIMEZONE, NTP_SERVER);

    if (!adjust_rtc()) {
        return false;
    }

    wifi_disconnect();

    return true;
}

bool rtc_set_unix_time(time_t unix_timestamp) {
    timeval tv = {
        .tv_sec = unix_timestamp,
        .tv_usec = 0,
    };

    settimeofday(&tv, nullptr);

    setenv("TZ", TIMEZONE, 1);
    tzset();

    if (!adjust_rtc()) {
        return false;
    }

    return true;
}
