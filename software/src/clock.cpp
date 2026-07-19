#include "RTClib.h"
#include "config.h"
#include "main.h"
#include "pins.h"
#include "rtc.h"

// Multiplexing config
#define SCAN_TICK_US 100
#define ANODE_ON_TICKS (2000 / SCAN_TICK_US)
#define CATHODE_OFF_DELAY_TICKS (400 / SCAN_TICK_US)
#define BLANK_TICKS (200 / SCAN_TICK_US)

// Neons config
#define NEON_PWM_FREQ_HZ 1000
#define NEON_PWM_RESOLUTION_BITS 10
#define NEON_PWM_MAX_DUTY ((1 << NEON_PWM_RESOLUTION_BITS) - 1)
#define NEON_PWM_CHANNEL_1 0
#define NEON_PWM_CHANNEL_2 1
#define NEON_BRIGHTNESS_PERCENT 60
#define NEON_HALF_PERIOD_MS 500

// RTC config
#define RTC_READ_INTERVAL_MS 50

static const uint8_t digit_count = sizeof(anode_pins) / sizeof(anode_pins[0]);

static hw_timer_t* scan_timer = nullptr;
static portMUX_TYPE display_mux = portMUX_INITIALIZER_UNLOCKED;

static volatile uint8_t display_digits[digit_count] = {};

RTC_DS3231 rtc;
static bool rtc_available = false;

static bool acp_routine_running = false;

static void set_display(uint32_t value) {
    uint8_t digits[digit_count];

    for (int i = 0; i < digit_count; i++) {
        int digit = value % 10;
        digits[i] = digit;
        value /= 10;
    }

    portENTER_CRITICAL(&display_mux);
    for (uint8_t i = 0; i < digit_count; i++) {
        display_digits[i] = digits[i];
    }
    portEXIT_CRITICAL(&display_mux);
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
    static uint8_t phase_ticks = 0;
    static ScanPhase phase = SET_CATHODE;

    switch (phase) {
        case SET_CATHODE:
            portENTER_CRITICAL_ISR(&display_mux);
            active_cathode = display_digits[active_anode];
            portEXIT_CRITICAL_ISR(&display_mux);

            digitalWrite(cathode_pins[active_cathode], HIGH);
            phase = ENABLE_ANODE;
            break;

        case ENABLE_ANODE:
            digitalWrite(anode_pins[active_anode], HIGH);
            phase_ticks = ANODE_ON_TICKS;
            phase = DISABLE_ANODE;
            break;

        case DISABLE_ANODE:
            if (--phase_ticks == 0) {
                digitalWrite(anode_pins[active_anode], LOW);
                phase_ticks = CATHODE_OFF_DELAY_TICKS;
                phase = CLEAR_CATHODE;
            }
            break;

        case CLEAR_CATHODE:
            if (--phase_ticks == 0) {
                digitalWrite(cathode_pins[active_cathode], LOW);
                phase_ticks = BLANK_TICKS;
                phase = NEXT_DIGIT;
            }
            break;

        case NEXT_DIGIT:
            if (--phase_ticks == 0) {
                active_anode = (active_anode + 1) % digit_count;
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

static uint32_t neon_duty_from_percent(uint8_t percent) {
    if (percent > 100) {
        percent = 100;
    }

    return (NEON_PWM_MAX_DUTY * percent) / 100;
}

static void set_neons_enabled(bool enabled) {
    uint32_t duty =
        enabled ? neon_duty_from_percent(NEON_BRIGHTNESS_PERCENT) : 0;

    ledcWrite(NEON_PWM_CHANNEL_1, duty);
    ledcWrite(NEON_PWM_CHANNEL_2, duty);
}

static void sync_neons_to_second_phase(uint32_t second_started_ms) {
    uint32_t elapsed_ms = millis() - second_started_ms;

    set_neons_enabled(elapsed_ms < NEON_HALF_PERIOD_MS);
}

static void setup_neon_pwm() {
    ledcSetup(NEON_PWM_CHANNEL_1, NEON_PWM_FREQ_HZ, NEON_PWM_RESOLUTION_BITS);
    ledcSetup(NEON_PWM_CHANNEL_2, NEON_PWM_FREQ_HZ, NEON_PWM_RESOLUTION_BITS);
    ledcAttachPin(PIN_NEON_1, NEON_PWM_CHANNEL_1);
    ledcAttachPin(PIN_NEON_2, NEON_PWM_CHANNEL_2);
    set_neons_enabled(false);
}

static void init_rtc() {
    start_scan_timer();

    rtc_available = rtc.begin(&Wire);
    if (!rtc_available) {
        Serial.println("DS3231 not found");
        set_display(999999);
        return;
    }

    if (rtc.lostPower()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    DateTime now = rtc.now();
    set_display(time_display_value(now));
    sync_neons_to_second_phase(millis());
}

static void anti_cathode_poisoning_routine() {
    if (!acp_routine_running) return;

    static uint32_t last_step_millis = millis();
    static uint8_t step = 0;

    if (millis() - last_step_millis < 100) return;
    last_step_millis = millis();

    if (step < 20) {
        uint32_t n = (step % 10) * 111111;

        set_display(n);

        step++;
    } else {
        acp_routine_running = false;
        step = 0;
    }
}

void clock_update() {
    anti_cathode_poisoning_routine();

    if (acp_routine_running) return;

    static uint32_t last_read_ms = 0;
    static uint32_t last_ntp_update = 0;
    static uint8_t last_second = UINT8_MAX;
    static uint32_t second_started_ms = 0;
    uint32_t now_ms = millis();

    if (rtc_available && now_ms - last_read_ms >= RTC_READ_INTERVAL_MS) {
        last_read_ms = now_ms;

        DateTime now = rtc.now();
        uint8_t now_s = now.second();

        set_hv_enabled(!tubes_should_sleep(now));

        if (now_s != last_second) {
            last_second = now.second();
            second_started_ms = now_ms;
            set_display(time_display_value(now));
        }

        if (now_s == 0) {
            acp_routine_running = true;
        }

        sync_neons_to_second_phase(second_started_ms);
    }

    if (config.automatic_time && now_ms - last_ntp_update >= 1000 * 60 * 60) {
        Serial.println("Setting time automatically...");
        last_ntp_update = now_ms;

        if (!rtc_ntp_fetch_time()) {
            Serial.println("Failed to set time automatically");
        }
    }
}

void clock_setup() {
    setup_pins(anode_pins, digit_count);
    setup_pins(cathode_pins, 10);
    setup_neon_pwm();

    init_rtc();
}
