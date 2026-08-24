#pragma once

#include <stdint.h>

extern const uint8_t clock_digit_count;

bool clock_setup();

void clock_update();

bool clock_acp_routine_running();

void clock_get_display_digits(uint8_t* digits);

void clock_set_display_digits(const uint8_t* digits);

void clock_set_display(uint32_t value);

bool clock_get_current_time_digits(uint8_t* digits);

void clock_stop_acp_routine();

void clock_start_acp_routine();

void clock_apply_neon_pwm_config(uint32_t frequency, uint16_t brightness);
