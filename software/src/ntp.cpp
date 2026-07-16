#include "RTClib.h"
#include "clock.h"
#include "time.h"
#include "wifi.h"

#define NTP_SERVER "pool.ntp.org"
#define TIMEZONE "CET-1CEST,M3.5.0/2,M10.5.0/3"

bool ntp_fetch_time() {
    if (!wifi_connect_with_config_credentials()) return false;

    Serial.print("Fetching time from NTP server ");
    Serial.println(NTP_SERVER);

    configTzTime(TIMEZONE, NTP_SERVER);

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to local obtain time.");
        return false;
    }

    int year = timeinfo.tm_year + 1900;
    int month = timeinfo.tm_mon + 1;
    int day = timeinfo.tm_mday;
    int hour = timeinfo.tm_hour;
    int minute = timeinfo.tm_min;
    int second = timeinfo.tm_sec;

    rtc.adjust(DateTime(year, month, day, hour, minute, second));
    Serial.print("Time adjusted to ");
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

    wifi_disconnect();

    return true;
}