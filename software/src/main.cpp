#include <Arduino.h>
#include <Wire.h>

#include "RTClib.h"

#define PIN_SDA 1
#define PIN_SCL 2
#define PIN_SHDN 15
#define PIN_NEON_1 9
#define PIN_NEON_2 8

#define SCAN_TICK_US 100
#define ANODE_ON_TICKS (1000 / SCAN_TICK_US)
#define CATHODE_OFF_DELAY_TICKS (400 / SCAN_TICK_US)
#define BLANK_TICKS (400 / SCAN_TICK_US)

static const uint8_t anode_pins[] = {21, 14, 13, 12, 11, 10};
static const uint8_t cathode_pins[] = {47, 48, 35, 36, 37, 38, 39, 40, 41, 42};
static const uint8_t digit_count = sizeof(anode_pins) / sizeof(anode_pins[0]);

static hw_timer_t* scan_timer = nullptr;
static portMUX_TYPE display_mux = portMUX_INITIALIZER_UNLOCKED;

static volatile uint8_t display_digits[digit_count] = {};

RTC_DS3231 rtc;
static bool rtc_available = false;

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

static uint32_t time_display_value(const DateTime& now) {
    return (now.hour() * 10000UL) + (now.minute() * 100UL) + now.second();
}

static void setup_pin(uint8_t pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

static void setup_pins(const uint8_t* pins, uint8_t count) {
    for (uint8_t i = 0; i < count; i++) {
        setup_pin(pins[i]);
    }
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

    set_display(time_display_value(rtc.now()));
}

static void anti_cathode_poisoning_routine() {
    for (uint8_t i = 0; i < 10; i++) {
        uint32_t n = i * 111111;

        set_display(n);
        delay(100);
    }
}

static void update_time_display() {
    static uint32_t last_read_ms = 0;
    static uint32_t last_neon_blink_ms = 0;
    static uint32_t last_anti_cathode_poisoning_ms = 0;
    static bool neon_enabled = false;

    if (!rtc_available || millis() - last_read_ms >= 100) {
        last_read_ms = millis();
        set_display(time_display_value(rtc.now()));
    }

    if (millis() - last_neon_blink_ms >= 500) {
        last_neon_blink_ms = millis();
        digitalWrite(PIN_NEON_1, neon_enabled);
        digitalWrite(PIN_NEON_2, neon_enabled);

        neon_enabled = !neon_enabled;
    }

    if (millis() - last_anti_cathode_poisoning_ms >= 60000) {
        last_anti_cathode_poisoning_ms = millis();

        anti_cathode_poisoning_routine();
    }
}

void setup() {
    setup_pin(PIN_NEON_1);
    setup_pin(PIN_NEON_2);
    setup_pins(anode_pins, digit_count);
    setup_pins(cathode_pins, 10);

    Serial.begin(9600);
    Wire.begin(PIN_SDA, PIN_SCL);

    init_rtc();

    delay(1000);
    pinMode(PIN_SHDN, OUTPUT);
    digitalWrite(PIN_SHDN, HIGH);
}

void loop() { update_time_display(); }
