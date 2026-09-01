#include "clock.h"

#include "RTClib.h"
#include "acp.h"
#include "config.h"
#include "main.h"
#include "pins.h"
#include "rtc.h"
#include "soc/gpio_struct.h"

// Multiplexing config
#define SCAN_TICK_US 50
#define ANODE_ON_TICKS (2000 / SCAN_TICK_US)
#define CATHODE_OFF_DELAY_TICKS (400 / SCAN_TICK_US)
#define BLANK_TICKS (200 / SCAN_TICK_US)
#define DIGIT_CROSS_FADE_DURATION_MS 250
#define DIGIT_CROSS_FADE_STEPS ANODE_ON_TICKS

// Neons config
#define NEON_PWM_RESOLUTION_BITS 10
#define NEON_PWM_MAX_DUTY ((1 << NEON_PWM_RESOLUTION_BITS) - 1)
#define NEON_PWM_CHANNEL_1 0
#define NEON_PWM_CHANNEL_2 1
#define NEON_HALF_PERIOD_MS 500

// RTC config
#define RTC_READ_INTERVAL_MS 50

// ACP config
#define ACP_ROUTINE_BASIC 0

extern const uint8_t clock_digit_count =
    sizeof(anode_pins) / sizeof(anode_pins[0]);

static hw_timer_t* scan_timer = nullptr;
static portMUX_TYPE display_mux = portMUX_INITIALIZER_UNLOCKED;

static volatile uint8_t display_digits[clock_digit_count] = {};
static volatile uint8_t display_cross_fade_from_digits[clock_digit_count] = {};
static volatile uint8_t display_cross_fade_step = DIGIT_CROSS_FADE_STEPS;

static bool acp_routine_running = false;
static bool digit_cross_fade_active = false;
static bool neons_enabled = false;

static uint8_t cycle_next_acp_routine = 0;
static uint32_t digit_cross_fade_started_ms = 0;

bool clock_acp_routine_running() { return acp_routine_running; }

static inline void IRAM_ATTR scan_gpio_write(uint8_t pin, bool high) {
    if (pin < 32) {
        uint32_t mask = 1UL << pin;

        if (high) {
            GPIO.out_w1ts = mask;
        } else {
            GPIO.out_w1tc = mask;
        }
    } else {
        uint32_t mask = 1UL << (pin - 32);

        if (high) {
            GPIO.out1_w1ts.data = mask;
        } else {
            GPIO.out1_w1tc.data = mask;
        }
    }
}

void clock_get_display_digits(uint8_t* digits) {
    portENTER_CRITICAL(&display_mux);
    for (uint8_t i = 0; i < clock_digit_count; i++) {
        digits[i] = display_digits[i];
    }
    portEXIT_CRITICAL(&display_mux);
}

static void cancel_digit_cross_fade() {
    digit_cross_fade_active = false;

    portENTER_CRITICAL(&display_mux);
    display_cross_fade_step = DIGIT_CROSS_FADE_STEPS;
    portEXIT_CRITICAL(&display_mux);
}

void clock_set_display_digits(const uint8_t* digits) {
    digit_cross_fade_active = false;

    portENTER_CRITICAL(&display_mux);
    for (uint8_t i = 0; i < clock_digit_count; i++) {
        display_digits[i] = digits[i];
        display_cross_fade_from_digits[i] = digits[i];
    }
    display_cross_fade_step = DIGIT_CROSS_FADE_STEPS;
    portEXIT_CRITICAL(&display_mux);
}

static void display_value_to_digits(uint32_t value, uint8_t* digits) {
    for (int i = 0; i < clock_digit_count; i++) {
        int digit = value % 10;
        digits[i] = digit;
        value /= 10;
    }
}

static bool display_digits_equal(const uint8_t* digits) {
    for (uint8_t i = 0; i < clock_digit_count; i++) {
        if (digits[i] != display_digits[i]) {
            return false;
        }
    }

    return true;
}

static void clock_cross_fade_display_digits(const uint8_t* digits) {
    portENTER_CRITICAL(&display_mux);

    if (display_digits_equal(digits)) {
        display_cross_fade_step = DIGIT_CROSS_FADE_STEPS;
        portEXIT_CRITICAL(&display_mux);
        digit_cross_fade_active = false;
        return;
    }

    for (uint8_t i = 0; i < clock_digit_count; i++) {
        display_cross_fade_from_digits[i] = display_digits[i];
        display_digits[i] = digits[i];
    }
    display_cross_fade_step = 0;

    portEXIT_CRITICAL(&display_mux);

    digit_cross_fade_started_ms = millis();
    digit_cross_fade_active = true;
}

static void update_digit_cross_fade(uint32_t now_ms) {
    if (!digit_cross_fade_active) {
        return;
    }

    if (!config.digit_cross_fade) {
        cancel_digit_cross_fade();
        return;
    }

    uint32_t elapsed_ms = now_ms - digit_cross_fade_started_ms;
    uint8_t step = DIGIT_CROSS_FADE_STEPS;

    if (elapsed_ms < DIGIT_CROSS_FADE_DURATION_MS) {
        step = (elapsed_ms * DIGIT_CROSS_FADE_STEPS) /
               DIGIT_CROSS_FADE_DURATION_MS;
    }

    portENTER_CRITICAL(&display_mux);
    display_cross_fade_step = step;
    portEXIT_CRITICAL(&display_mux);

    if (step >= DIGIT_CROSS_FADE_STEPS) {
        digit_cross_fade_active = false;
    }
}

void clock_set_display(uint32_t value) {
    uint8_t digits[clock_digit_count];

    display_value_to_digits(value, digits);

    if (config.digit_cross_fade && !acp_routine_running) {
        clock_cross_fade_display_digits(digits);
        return;
    }

    clock_set_display_digits(digits);
}

static uint32_t neon_duty_from_percent(uint16_t percent) {
    if (percent > 100) {
        percent = 100;
    }

    return (NEON_PWM_MAX_DUTY * percent) / 100;
}

static void write_neon_duty(bool enabled, uint16_t brightness) {
    uint32_t duty = enabled ? neon_duty_from_percent(brightness) : 0;

    ledcWrite(NEON_PWM_CHANNEL_1, duty);
    ledcWrite(NEON_PWM_CHANNEL_2, duty);
}

static void set_neons_enabled(bool enabled) {
    neons_enabled = enabled;
    write_neon_duty(enabled, config.neons_brightness);
}

void clock_stop_acp_routine() {
    acp_routine_running = false;

    if (!config.healing_mode && config.acp_routine == -1 &&
        acp_routine_count > 0) {
        cycle_next_acp_routine =
            (cycle_next_acp_routine + 1) % acp_routine_count;
    }
}

void clock_start_acp_routine() {
    cancel_digit_cross_fade();
    acp_routine_running = true;
    set_neons_enabled(false);
}

static void IRAM_ATTR scan_isr() {
    enum ScanPhase : uint8_t {
        SET_CATHODE,
        ENABLE_ANODE,
        DISABLE_ANODE,
        CLEAR_CATHODE,
        NEXT_DIGIT,
    };

    static uint8_t active_anode = 0;
    static uint8_t active_cathode = 0;
    static uint8_t active_cross_fade_to_cathode = 0;
    static uint8_t active_anode_on_ticks = ANODE_ON_TICKS;
    static uint8_t cross_fade_to_ticks = 0;
    static uint8_t phase_ticks = 0;
    static bool showing_cross_fade_from_digit = false;
    static ScanPhase phase = SET_CATHODE;

    switch (phase) {
        case SET_CATHODE:
            portENTER_CRITICAL_ISR(&display_mux);
            {
                uint8_t from_cathode =
                    display_cross_fade_from_digits[active_anode];
                uint8_t to_cathode = display_digits[active_anode];
                uint8_t fade_step = display_cross_fade_step;

                if (fade_step >= DIGIT_CROSS_FADE_STEPS ||
                    from_cathode == to_cathode) {
                    active_cathode = to_cathode;
                    active_cross_fade_to_cathode = to_cathode;
                    active_anode_on_ticks = ANODE_ON_TICKS;
                    cross_fade_to_ticks = 0;
                    showing_cross_fade_from_digit = false;
                } else {
                    cross_fade_to_ticks =
                        (ANODE_ON_TICKS * fade_step) / DIGIT_CROSS_FADE_STEPS;
                    uint8_t from_ticks = ANODE_ON_TICKS - cross_fade_to_ticks;

                    active_cathode = from_cathode;
                    active_cross_fade_to_cathode = to_cathode;
                    active_anode_on_ticks = from_ticks;
                    showing_cross_fade_from_digit = true;
                }
            }
            portEXIT_CRITICAL_ISR(&display_mux);

            scan_gpio_write(cathode_pins[active_cathode], HIGH);
            phase = ENABLE_ANODE;
            break;

        case ENABLE_ANODE:
            scan_gpio_write(anode_pins[active_anode], HIGH);
            phase_ticks = active_anode_on_ticks;
            phase = DISABLE_ANODE;
            break;

        case DISABLE_ANODE:
            if (--phase_ticks == 0) {
                scan_gpio_write(anode_pins[active_anode], LOW);
                phase_ticks = CATHODE_OFF_DELAY_TICKS;
                phase = CLEAR_CATHODE;
            }
            break;

        case CLEAR_CATHODE:
            if (--phase_ticks == 0) {
                scan_gpio_write(cathode_pins[active_cathode], LOW);

                if (showing_cross_fade_from_digit && cross_fade_to_ticks > 0) {
                    active_cathode = active_cross_fade_to_cathode;
                    active_anode_on_ticks = cross_fade_to_ticks;
                    showing_cross_fade_from_digit = false;

                    scan_gpio_write(cathode_pins[active_cathode], HIGH);
                    phase = ENABLE_ANODE;
                    break;
                }

                phase_ticks = BLANK_TICKS;
                phase = NEXT_DIGIT;
            }
            break;

        case NEXT_DIGIT:
            if (--phase_ticks == 0) {
                active_anode = (active_anode + 1) % clock_digit_count;
                phase = SET_CATHODE;
            }
            break;
    }
}

static void start_scan_timer() {
    scan_timer = timerBegin(0, 80, true);
    timerAttachInterrupt(scan_timer, &scan_isr, true);
    timerAlarmWrite(scan_timer, SCAN_TICK_US, true);
    timerAlarmEnable(scan_timer);
}

static uint16_t minutes_since_midnight(uint8_t hours, uint8_t minutes) {
    return (hours * 60U) + minutes;
}

static bool tubes_should_sleep(const DateTime& now) {
    if (!config.timer) {
        return false;
    }

    uint16_t current_time = minutes_since_midnight(now.hour(), now.minute());
    uint16_t off_time = minutes_since_midnight(config.timer_tubes_off_hours,
                                               config.timer_tubes_off_minutes);
    uint16_t on_time = minutes_since_midnight(config.timer_tubes_on_hours,
                                              config.timer_tubes_on_minutes);

    if (off_time == on_time) {
        return false;
    }

    if (off_time < on_time) {
        return current_time >= off_time && current_time < on_time;
    }

    return current_time >= off_time || current_time < on_time;
}

static uint32_t time_display_value(const DateTime& now) {
    uint8_t hour = now.hour();

    if (config.time_display_format == 12) {
        hour %= 12;
        if (hour == 0) {
            hour = 12;
        }
    }

    return (hour * 10000UL) + (now.minute() * 100UL) + now.second();
}

bool clock_get_current_time_digits(uint8_t* digits) {
    if (!rtc_is_available()) {
        return false;
    }

    display_value_to_digits(time_display_value(rtc.now()), digits);
    return true;
}

static bool neons_should_be_enabled(const DateTime& now,
                                    uint32_t second_started_ms) {
    if (acp_routine_running) return false;

    uint32_t elapsed_ms = millis() - second_started_ms;

    switch (config.neons_mode) {
        case NeonsMode::NEONS_BLINK:
            return elapsed_ms < NEON_HALF_PERIOD_MS;

        case NeonsMode::NEONS_TOGGLE:
            return now.second() % 2 == 0;

        case NeonsMode::NEONS_DISABLED:
        default:
            return false;
    }
}

static void sync_neons(const DateTime& now, uint32_t second_started_ms) {
    set_neons_enabled(neons_should_be_enabled(now, second_started_ms));
}

void clock_apply_neon_pwm_config(uint32_t frequency, uint16_t brightness) {
    ledcSetup(NEON_PWM_CHANNEL_1, frequency, NEON_PWM_RESOLUTION_BITS);
    ledcSetup(NEON_PWM_CHANNEL_2, frequency, NEON_PWM_RESOLUTION_BITS);
    write_neon_duty(neons_enabled, brightness);
}

static void setup_neon_pwm() {
    clock_apply_neon_pwm_config(config.neons_frequency,
                                config.neons_brightness);
    ledcAttachPin(PIN_NEON_1, NEON_PWM_CHANNEL_1);
    ledcAttachPin(PIN_NEON_2, NEON_PWM_CHANNEL_2);
    set_neons_enabled(false);
}

void clock_update() {
    if (acp_routine_running) {
        int8_t routine =
            config.healing_mode ? ACP_ROUTINE_BASIC : config.acp_routine;
        if (!config.healing_mode && routine == -1) {
            routine = cycle_next_acp_routine;
        }

        if (routine < 0 || routine >= acp_routine_count) {
            routine = 0;
        }

        acp_routines[routine].run();
    }

    if (acp_routine_running) return;

    if (config.healing_mode) {
        clock_start_acp_routine();
        return;
    }

    static uint32_t last_read_ms = 0;
    static uint32_t last_ntp_update = 0;
    static uint8_t last_second = UINT8_MAX;
    static uint32_t second_started_ms = 0;
    uint32_t now_ms = millis();

    update_digit_cross_fade(now_ms);

    if (rtc_is_available() && now_ms - last_read_ms >= RTC_READ_INTERVAL_MS) {
        last_read_ms = now_ms;

        DateTime now = rtc.now();
        uint8_t now_s = now.second();

        set_hv_enabled(!tubes_should_sleep(now));

        if (now_s != last_second) {
            last_second = now_s;
            second_started_ms = now_ms;
            clock_set_display(time_display_value(now));
        }

        if (now_s == 0) {
            clock_start_acp_routine();
        }

        sync_neons(now, second_started_ms);
    }

    if (rtc_is_available() && config.automatic_time &&
        now_ms - last_ntp_update >= 60000UL * config.ntp_frequency) {
        Serial.println("Setting time automatically...");
        last_ntp_update = now_ms;

        if (!rtc_ntp_fetch_time()) {
            Serial.println("Failed to set time automatically");
        }
    }
}

bool clock_setup() {
    setup_pins(anode_pins, clock_digit_count);
    setup_pins(cathode_pins, 10);
    setup_neon_pwm();

    start_scan_timer();

    if (!rtc_init()) {
        return false;
    }

    DateTime now = rtc.now();
    clock_set_display(time_display_value(now));
    sync_neons(now, millis());

    return true;
}
