#include "acp.h"

#include <Arduino.h>

#include "clock.h"

#define ACP_STEP_INTERVAL_MS 50
#define ACP_DIGIT_VALUES 10
#define ACP_SLOT_STEPS_PER_DIGIT ACP_DIGIT_VALUES

static void get_base_display_digits(uint8_t* digits) {
    if (!clock_get_current_time_digits(digits)) {
        clock_get_display_digits(digits);
    }
}

static void anti_cathode_poisoning_routine_basic() {
    static uint32_t last_step_millis = millis();
    static uint8_t step = 0;
    static bool forwards = true;

    if (millis() - last_step_millis < ACP_STEP_INTERVAL_MS) return;
    last_step_millis = millis();

    if (step < 20) {
        uint8_t animation_step = (step % 10);
        if (!forwards) {
            animation_step = 9 - animation_step;
        }
        uint32_t n = animation_step * 111111;

        clock_set_display(n);

        step++;
    } else {
        clock_stop_acp_routine();
        step = 0;
        forwards = !forwards;
    }
}

static void anti_cathode_poisoning_routine_sweep() {
    static uint32_t last_step_millis = millis();
    static uint8_t step = 0;
    static bool left_to_right = true;

    if (millis() - last_step_millis < ACP_STEP_INTERVAL_MS) return;
    last_step_millis = millis();

    uint8_t digits[clock_digit_count];

    uint8_t sweep_duration = (clock_digit_count - 1) * 5 + 20;

    if (step >= sweep_duration) {
        get_base_display_digits(digits);
        clock_set_display_digits(digits);
        clock_stop_acp_routine();
        step = 0;
        left_to_right = !left_to_right;
        return;
    }

    get_base_display_digits(digits);

    for (uint8_t i = 0; i < clock_digit_count; i++) {
        uint8_t digit_index = left_to_right ? i : clock_digit_count - i - 1;
        uint8_t new_digit = digits[digit_index];

        if (step >= i * 5 && step < i * 5 + 20) {
            uint8_t active_step = step - i * 5;
            new_digit = (new_digit + active_step + 1) % 10;
        }

        digits[digit_index] = new_digit;
    }

    clock_set_display_digits(digits);
    step++;
}

static void anti_cathode_poisoning_routine_additive() {
    static uint32_t last_step_millis = millis();
    static uint8_t step = 0;
    static bool forwards = true;

    if (millis() - last_step_millis < ACP_STEP_INTERVAL_MS) return;
    last_step_millis = millis();

    if (step >= 20) {
        clock_stop_acp_routine();
        step = 0;
        forwards = !forwards;
        return;
    }

    uint8_t digits[clock_digit_count];

    get_base_display_digits(digits);

    for (uint8_t i = 0; i < clock_digit_count; i++) {
        uint8_t new_digit = digits[i];
        uint8_t offset = (step + 1) % ACP_DIGIT_VALUES;

        if (forwards) {
            new_digit = (new_digit + offset) % ACP_DIGIT_VALUES;
        } else {
            new_digit =
                (new_digit + ACP_DIGIT_VALUES - offset) % ACP_DIGIT_VALUES;
        }

        digits[i] = new_digit;
    }

    clock_set_display_digits(digits);
    step++;
}

static void anti_cathode_poisoning_routine_slot_machine() {
    static uint32_t last_step_millis = millis();
    static uint8_t step = 0;
    static bool left_to_right = true;

    if (millis() - last_step_millis < ACP_STEP_INTERVAL_MS) return;
    last_step_millis = millis();

    uint8_t locked_digits = step / ACP_SLOT_STEPS_PER_DIGIT;
    uint8_t target_digits[clock_digit_count];

    get_base_display_digits(target_digits);

    if (locked_digits >= clock_digit_count) {
        clock_set_display_digits(target_digits);
        clock_stop_acp_routine();
        left_to_right = !left_to_right;
        step = 0;
        return;
    }

    uint8_t digits[clock_digit_count];

    for (uint8_t i = 0; i < clock_digit_count; i++) {
        uint8_t digit_index = left_to_right ? clock_digit_count - i - 1 : i;

        if (i < locked_digits) {
            digits[digit_index] = target_digits[digit_index];
        } else {
            digits[digit_index] = (step + digit_index * 3) % ACP_DIGIT_VALUES;
        }
    }

    clock_set_display_digits(digits);
    step++;
}

static void anti_cathode_poisoning_routine_centroid() {
    static uint32_t last_step_millis = millis();
    static uint8_t step = 0;
    static bool inwards = true;

    if (millis() - last_step_millis < ACP_STEP_INTERVAL_MS) return;
    last_step_millis = millis();

    uint8_t digits[clock_digit_count];

    get_base_display_digits(digits);

    uint8_t center = clock_digit_count / 2;

    for (uint8_t i = 0; i < clock_digit_count / 2; i++) {
        uint8_t i1 = inwards ? i : center - i - 1;
        uint8_t i2 = inwards ? clock_digit_count - i - 1 : center + i;

        uint8_t d1 = digits[i1];
        uint8_t d2 = digits[i2];

        if (step >= i * 5 && step < i * 5 + 20) {
            uint8_t active_step = step - i * 5;
            d1 = (d1 + active_step + 1) % 10;
            d2 = (d2 + active_step + 1) % 10;
        }

        digits[i1] = d1;
        digits[i2] = d2;
    }

    clock_set_display_digits(digits);

    if (step < clock_digit_count / 2 * 10) {
        step++;
    } else {
        clock_stop_acp_routine();
        step = 0;
        inwards = !inwards;
    }
}

const AcpRoutine acp_routines[] = {
    {anti_cathode_poisoning_routine_basic},
    {anti_cathode_poisoning_routine_sweep},
    {anti_cathode_poisoning_routine_additive},
    {anti_cathode_poisoning_routine_slot_machine},
    {anti_cathode_poisoning_routine_centroid}};

const uint8_t acp_routine_count =
    sizeof(acp_routines) / sizeof(acp_routines[0]);
