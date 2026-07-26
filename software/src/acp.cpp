#include "acp.h"

#include <Arduino.h>

#include "clock.h"

#define ACP_STEP_INTERVAL_MS 50

static void anti_cathode_poisoning_routine_basic() {
    static uint32_t last_step_millis = millis();
    static uint8_t step = 0;

    if (millis() - last_step_millis < ACP_STEP_INTERVAL_MS) return;
    last_step_millis = millis();

    if (step < 20) {
        uint32_t n = (step % 10) * 111111;

        clock_set_display(n);

        step++;
    } else {
        clock_stop_acp_routine();
        step = 0;
    }
}

static void anti_cathode_poisoning_routine_sweep() {
    static uint32_t last_step_millis = millis();
    static uint8_t step = 0;
    static bool left_to_right = true;

    if (millis() - last_step_millis < ACP_STEP_INTERVAL_MS) return;
    last_step_millis = millis();

    uint8_t digits[clock_digit_count];

    clock_get_display_digits(digits);

    for (uint8_t i = 0; i < clock_digit_count; i++) {
        uint8_t digit_index = left_to_right ? i : clock_digit_count - i - 1;
        uint8_t new_digit = digits[digit_index];

        if (step >= i * 5 && step < i * 5 + 20) {
            new_digit = (new_digit + 1) % 10;
        }

        digits[digit_index] = new_digit;
    }

    clock_set_display_digits(digits);

    if (step < clock_digit_count * 10) {
        step++;
    } else {
        clock_stop_acp_routine();
        step = 0;
        left_to_right = !left_to_right;
    }
}

static void anti_cathode_poisoning_routine_additive() {
    static uint32_t last_step_millis = millis();
    static uint8_t step = 0;

    if (millis() - last_step_millis < ACP_STEP_INTERVAL_MS) return;
    last_step_millis = millis();

    uint8_t digits[clock_digit_count];

    clock_get_display_digits(digits);

    for (uint8_t i = 0; i < clock_digit_count; i++) {
        uint8_t new_digit = digits[i];
        new_digit = (new_digit + 1) % 10;

        digits[i] = new_digit;
    }

    clock_set_display_digits(digits);

    if (step < 20) {
        step++;
    } else {
        clock_stop_acp_routine();
        step = 0;
    }
}

const AcpRoutine acp_routines[] = {{anti_cathode_poisoning_routine_basic},
                                   {anti_cathode_poisoning_routine_sweep},
                                   {anti_cathode_poisoning_routine_additive}};

const uint8_t acp_routine_count = sizeof(acp_routines) / sizeof(acp_routines[0]);
