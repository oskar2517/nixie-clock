#pragma once

#include <stdint.h>

#define PIN_SDA 1
#define PIN_SCL 2

#define PIN_SHDN 15
#define PIN_NEON_1 9
#define PIN_NEON_2 8

const uint8_t anode_pins[] = {21, 14, 13, 12, 11, 10};
const uint8_t cathode_pins[] = {47, 48, 35, 36, 37, 38, 39, 40, 41, 42};

void setup_pin(uint8_t pin);

void setup_pins(const uint8_t* pins, uint8_t count);