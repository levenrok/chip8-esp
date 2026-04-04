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

static inline void core_increment(Core* self) {
    self->pc += 2;
}

void core_cycle(Core* self) {
    self->opcode = self->memory[self->pc] << 8 | self->memory[self->pc + 1];

    ESP_COMPILER_DIAGNOSTIC_PUSH_IGNORE("-Wanalyzer-symbol-too-complex")
    const u8 instruction = (self->opcode & 0xF000) >> 12;
    const u16 nnn = self->opcode & 0x0FFF;
    const u8 n = self->opcode & 0x000F;
    const u8 x = (self->opcode & 0x0F00) >> 8;
    const u8 y = (self->opcode & 0x00F0) >> 4;
    const u8 kk = self->opcode & 0x00FF;
    ESP_COMPILER_DIAGNOSTIC_POP()

    const u8 vx = self->v[x];
    const u8 vy = self->v[y];

    ESP_LOGI(TAG, "instruction: 0x%02X, opcode: 0x%04X, counter: 0x%04X",
             instruction, self->opcode, self->pc);

    switch (instruction) {
        case 0x0:
            if (kk == 0xE0) {
                (void)memset(self->graphics, 0, GRAPHICS_SIZE);
            } else if (kk == 0xEE) {
                self->pc = self->stack[self->sp];
                self->sp -= 1;
            }
            core_increment(self);
            break;

        case 0x1:
            self->pc = nnn;
            break;

        case 0x2:
            self->sp += 1;
            self->stack[self->sp] = self->pc;
            self->pc = nnn;
            break;

        case 0x3:
            if (vx == kk)
                core_increment(self);

            core_increment(self);
            break;

        case 0x4:
            if (vx != kk)
                core_increment(self);

            core_increment(self);
            break;

        case 0x5:
            if (vx == vy)
                core_increment(self);

            core_increment(self);
            break;

        case 0x6:
            self->v[x] = kk;
            core_increment(self);
            break;

        case 0x7:
            self->v[x] = vx + kk;
            core_increment(self);
            break;

        case 0x8: {
            switch (n) {
                case 0x0:
                    self->v[x] = vy;
                    break;

                case 0x1:
                    self->v[x] |= vy;
                    break;

                case 0x2:
                    self->v[x] &= vy;
                    break;

                case 0x3:
                    self->v[x] ^= vy;
                    break;

                case 0x4: {
                    u8 result = 0;
                    bool overflowing = __builtin_add_overflow(vx, vy, &result);

                    self->v[0xF] = overflowing ? 1 : 0;
                    self->v[x] = result;
                } break;

                case 0x5:
                    self->v[0xF] = vx > vy ? 1 : 0;
                    self->v[x] -= vy;
                    break;

                case 0x6:
                    self->v[0xF] = vx << 1 == 1 ? 1 : 0;
                    self->v[x] /= 2;
                    break;

                case 0x7:
                    self->v[0xF] = vy > vx ? 1 : 0;
                    self->v[x] = vy - vx;
                    break;

                case 0xE:
                    self->v[0xF] = vx >> 1 == 1 ? 1 : 0;
                    self->v[x] *= 2;
                    break;

                default:
                    goto unknown;
            }
            core_increment(self);
        } break;

        default:
            goto unknown;
    }

    return;

unknown:
    ESP_LOGE(TAG, "unknown instruction! (0x%X)", instruction);
    return;
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
