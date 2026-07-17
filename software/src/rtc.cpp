#include <sys/time.h>
#include <time.h>

#include "RTClib.h"
#include "clock.h"
#include "config.h"
#include "wifi.h"

#define NTP_SERVER "pool.ntp.org"

static tm tm_from_datetime(const DateTime& datetime) {
    tm timeinfo = {};
    timeinfo.tm_year = datetime.year() - 1900;
    timeinfo.tm_mon = datetime.month() - 1;
    timeinfo.tm_mday = datetime.day();
    timeinfo.tm_hour = datetime.hour();
    timeinfo.tm_min = datetime.minute();
    timeinfo.tm_sec = datetime.second();
    timeinfo.tm_isdst = -1;

    return timeinfo;
}

static void set_system_time(time_t unix_timestamp) {
    timeval tv = {
        .tv_sec = unix_timestamp,
        .tv_usec = 0,
    };

    settimeofday(&tv, nullptr);
}

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

static bool set_timezone(const char* posix_timezone) {
    if (!posix_timezone) {
        return false;
    }

    setenv("TZ", posix_timezone, 1);
    tzset();
    return true;
}

bool rtc_ntp_fetch_time() {
    if (!wifi_connect_with_config_credentials()) return false;

    Serial.print("Fetching time from NTP server ");
    Serial.println(NTP_SERVER);

    configTzTime(config.timezone_posix.c_str(), NTP_SERVER);

    if (!adjust_rtc()) {
        wifi_disconnect();
        return false;
    }

    wifi_disconnect();

    return true;
}

bool rtc_set_unix_time(time_t unix_timestamp) {
    set_system_time(unix_timestamp);

    if (!set_timezone(config.timezone_posix.c_str())) {
        return false;
    }

    if (!adjust_rtc()) {
        return false;
    }

    return true;
}

bool rtc_set_timezone(const char* posix_timezone) {
    const char* current_posix_timezone = config.timezone_posix.c_str();

    if (!set_timezone(current_posix_timezone)) {
        return false;
    }

    tm current_time = tm_from_datetime(rtc.now());
    time_t unix_timestamp = mktime(&current_time);
    if (unix_timestamp == (time_t)-1) {
        return false;
    }

    set_system_time(unix_timestamp);

    if (!set_timezone(posix_timezone)) {
        return false;
    }

    return adjust_rtc();
}
