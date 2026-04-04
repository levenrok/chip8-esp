#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "esp_compiler.h"
#include "esp_log.h"

#include "include/core.h"
#include "include/rom.h"

#define MEMORY_SIZE MAX_ROM_SIZE + 512
#define GRAPHICS_SIZE 64 * 32

static const char TAG[] = "core";
static const u8 FONT_SET[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80,  // F
};

Core core_init(const Rom* rom) {
    if (!rom_check(rom))
        goto err;

    const u16 offset = 0x0200;

    Core core = {
        .memory = calloc(MEMORY_SIZE, sizeof(u8)),
        .v = {0},
        .i = 0,
        .opcode = 0,
        .pc = offset,
        .stack = {0},
        .sp = 0,
        .keys = {0},
        .graphics = calloc(GRAPHICS_SIZE, sizeof(u8)),
        .delay_timer = 0,
        .sound_timer = 0,
    };

    if (core.memory == NULL) {
        free(core.graphics);
        goto err;
    } else if (core.graphics == NULL) {
        free(core.memory);
        goto err;
    }

    (void)memcpy(core.memory, FONT_SET, sizeof(FONT_SET));
    (void)memcpy(core.memory + offset, rom->data, rom->size);

    return core;

err:
    return (Core){0};
}

void core_deinit(Core* self) {
    if (self->memory != NULL) {
        (void)free(self->memory);
        self->memory = NULL;

        if (self->graphics != NULL) {
            (void)free(self->graphics);
            self->graphics = NULL;
        }
    }
}
