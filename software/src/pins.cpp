#include <Arduino.h>
#include <stdint.h>

void setup_pin(uint8_t pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void setup_pins(const uint8_t* pins, uint8_t count) {
    for (uint8_t i = 0; i < count; i++) {
        setup_pin(pins[i]);
    }
}