#pragma once

#include <stdint.h>

typedef struct {
    void (*run)();
} AcpRoutine;

extern const AcpRoutine acp_routines[];

extern const uint8_t acp_routine_count;
