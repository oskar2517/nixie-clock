#include "RTClib.h"
#include "acp.h"
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

RTC_DS3231 rtc;
static bool rtc_available = false;

static bool acp_routine_running = false;
static bool neons_enabled = false;

static uint8_t cycle_next_acp_routine = 0;

bool clock_rtc_available() { return rtc_available; }

bool clock_acp_routine_running() { return acp_routine_running; }

void clock_get_display_digits(uint8_t* digits) {
    portENTER_CRITICAL(&display_mux);
    for (uint8_t i = 0; i < clock_digit_count; i++) {
        digits[i] = display_digits[i];
    }
    portEXIT_CRITICAL(&display_mux);
}

void clock_set_display_digits(const uint8_t* digits) {
    portENTER_CRITICAL(&display_mux);
    for (uint8_t i = 0; i < clock_digit_count; i++) {
        display_digits[i] = digits[i];
    }
    portEXIT_CRITICAL(&display_mux);
}

void clock_set_display(uint32_t value) {
    uint8_t digits[clock_digit_count];

    for (int i = 0; i < clock_digit_count; i++) {
        int digit = value % 10;
        digits[i] = digit;
        value /= 10;
    }

    clock_set_display_digits(digits);
}

void clock_stop_acp_routine() {
    acp_routine_running = false;

    if (!config.healing_mode && config.acp_routine == -1 &&
        acp_routine_count > 0) {
        cycle_next_acp_routine =
            (cycle_next_acp_routine + 1) % acp_routine_count;
    }
}

void clock_start_acp_routine() { acp_routine_running = true; }

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

static uint32_t neon_duty_from_percent(uint16_t percent) {
    if (percent > 100) {
        percent = 100;
    }

    return (NEON_PWM_MAX_DUTY * percent) / 100;
}

static void set_neons_enabled(bool enabled) {
    neons_enabled = enabled;

    uint32_t duty =
        enabled ? neon_duty_from_percent(config.neons_brightness) : 0;

    ledcWrite(NEON_PWM_CHANNEL_1, duty);
    ledcWrite(NEON_PWM_CHANNEL_2, duty);
}

static bool neons_should_be_enabled(const DateTime& now,
                                    uint32_t second_started_ms) {
    uint32_t elapsed_ms = millis() - second_started_ms;

    switch (config.neons_mode) {
        case CFG_NEONS_MODE_BLINK:
            return elapsed_ms < NEON_HALF_PERIOD_MS;

        case CFG_NEONS_MODE_TOGGLE:
            return now.second() % 2 == 0;

        case CFG_NEONS_MODE_DISABLED:
        default:
            return false;
    }
}

static void sync_neons(const DateTime& now, uint32_t second_started_ms) {
    set_neons_enabled(neons_should_be_enabled(now, second_started_ms));
}

void clock_apply_neon_pwm_config() {
    ledcSetup(NEON_PWM_CHANNEL_1, config.neons_frequency,
              NEON_PWM_RESOLUTION_BITS);
    ledcSetup(NEON_PWM_CHANNEL_2, config.neons_frequency,
              NEON_PWM_RESOLUTION_BITS);
    set_neons_enabled(neons_enabled);
}

static void setup_neon_pwm() {
    clock_apply_neon_pwm_config();
    ledcAttachPin(PIN_NEON_1, NEON_PWM_CHANNEL_1);
    ledcAttachPin(PIN_NEON_2, NEON_PWM_CHANNEL_2);
    set_neons_enabled(false);
}

static void init_rtc() {
    start_scan_timer();

    rtc_available = rtc.begin(&Wire);
    delay(100); // Make sure to wait until RTC is available...
    if (!rtc_available) {
        Serial.println("DS3231 not found");
        clock_set_display(999999);
        return;
    }

    if (rtc.lostPower()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    DateTime now = rtc.now();
    clock_set_display(time_display_value(now));
    sync_neons(now, millis());
}

void clock_update() {
    if (acp_routine_running) {
        int8_t routine = config.healing_mode ? ACP_ROUTINE_BASIC
                                             : config.acp_routine;
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
        // Only use basic acp mode for healing
        set_neons_enabled(false);
        clock_start_acp_routine();
        return;
    }

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
            clock_set_display(time_display_value(now));
        }

        if (now_s == 0) {
            clock_start_acp_routine();
        }

        sync_neons(now, second_started_ms);
    }

    if (config.automatic_time &&
        now_ms - last_ntp_update >= 60000UL * config.ntp_frequency) {
        Serial.println("Setting time automatically...");
        last_ntp_update = now_ms;

        if (!rtc_ntp_fetch_time()) {
            Serial.println("Failed to set time automatically");
        }
    }
}

void clock_setup() {
    setup_pins(anode_pins, clock_digit_count);
    setup_pins(cathode_pins, 10);
    setup_neon_pwm();

    init_rtc();
}
