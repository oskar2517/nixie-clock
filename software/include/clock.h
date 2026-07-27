#pragma once

#include <stdint.h>

#include "RTClib.h"

extern RTC_DS3231 rtc;

extern const uint8_t clock_digit_count;

void clock_setup();

void clock_update();

bool clock_rtc_available();

bool clock_acp_routine_running();

void clock_get_display_digits(uint8_t* digits);

void clock_set_display_digits(const uint8_t* digits);

void clock_set_display(uint32_t value);

void clock_stop_acp_routine();

void clock_start_acp_routine();

void clock_set_neon_pwm_frequency(uint32_t frequency);
