#pragma once

#include <stdbool.h>

#include "common.h"
#include "rom.h"

typedef struct {
    u8* memory;
    u8 v[16];
    u16 i;
    u16 opcode;
    u16 pc;
    u16 stack[16];
    u16 sp;
    u8 keys[16];
    u8* graphics;
    u8 delay_timer;
    u8 sound_timer;
} Core;

Core core_init(const Rom* rom);
void core_cycle(Core* self);
void core_deinit(Core* self);

static inline bool core_check(const Core* self) {
    if (self->memory == NULL || self->pc == 0 || self->graphics == NULL)
        return false;

    return true;
}
