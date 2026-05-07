#include <stddef.h>
#include <string.h>

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_random.h"

#include "include/core.h"

#define MAX_CORES 1

typedef struct Core {
    u8* memory;
    u8* graphics;
    u16 stack[16];
    u16 i;
    u16 opcode;
    u16 pc;
    u16 sp;
    u8 v[16];
    u8 keys[16];
    u8 delay_timer;
    u8 sound_timer;

    bool loaded;
} Core;

typedef struct Object {
    Core object;
    bool allocated;
} Object;

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

static Object core_pool[MAX_CORES];

void core_increment(Core* self) {
    self->pc += 2U;
}

ptrdiff_t object_index(CoreHandle handle) {
    return handle - (Core*)core_pool;
}

CoreHandle Core_Init(void) {
    if (core_pool[0].allocated) {
        ESP_LOGE(TAG, "core already initialized!");
        return NULL;
    }

    ESP_LOGI(TAG, "initializing emulator core...");

    CoreHandle core = &(core_pool[0].object);

    core->memory = (u8*)calloc(MEMORY_SIZE, sizeof(u8));
    core->graphics =
        (u8*)heap_caps_calloc(GRAPHICS_SIZE, sizeof(u8), MALLOC_CAP_DMA);

    if (core->memory == NULL) {
        free(core->graphics);

        ESP_LOGE(TAG, "failed to initialize core memory!");
        return NULL;
    } else if (core->graphics == NULL) {
        heap_caps_free(core->memory);

        ESP_LOGE(TAG, "failed to initialize core graphics!");
        return NULL;
    }

    core->pc = 0x0200U;

    core_pool[0].allocated = true;

    ESP_LOGI(TAG, "core initialized.");
    return core;
}

bool Core_Load(CoreHandle self, const char* name, const u8* data, usize size) {
    if (self == NULL) {
        ESP_LOGE(TAG, "core has not been successfully initialized!");
        return false;
    }

    ESP_LOGI(TAG, "loading ROM: name: %s, size: %d", name, size);

    if (size > MAX_ROM_SIZE) {
        ESP_LOGE(TAG, "failed to load ROM! (max: %d bytes, current: %d bytes)",
                 MAX_ROM_SIZE, size);
        return false;
    }

    (void)memcpy(self->memory, FONT_SET, sizeof(FONT_SET));
    (void)memcpy(self->memory + 0x0200, data, size);

    self->loaded = true;

    ESP_LOGI(TAG, "ROM loaded.");
    return true;
}

bool Core_Cycle(CoreHandle self) {
    if (self == NULL) {
        return false;
    } else if (!self->loaded) {
        ESP_LOGE(TAG, "no ROM file loaded!");
        return false;
    }

    self->opcode =
        (u16)(self->memory[self->pc] << 8U) | self->memory[self->pc + 1U];

    const u8 random_byte = (u8)esp_random();

    const u8 instruction = (self->opcode & 0xF000U) >> 12U;
    const u16 nnn = self->opcode & 0x0FFFU;
    const u8 n = self->opcode & 0x000FU;
    const u8 x = (self->opcode & 0x0F00U) >> 8U;
    const u8 y = (self->opcode & 0x00F0U) >> 4U;
    const u8 kk = self->opcode & 0x00FFU;

    const u8 vx = self->v[x];
    const u8 vy = self->v[y];

    ESP_LOGI(TAG, "instruction: 0x%02X, self->opcode: 0x%04X, counter: 0x%04X",
             instruction, self->opcode, self->pc);

    switch (instruction) {
        case 0x0:
            if (kk == 0xE0) {
                (void)memset(self->graphics, 0, GRAPHICS_SIZE);
            } else if (kk == 0xEE) {
                self->pc = self->stack[self->sp];
                self->sp -= 1U;
            }
            core_increment(self);
            break;

        case 0x1:
            self->pc = nnn;
            break;

        case 0x2:
            self->sp += 1U;
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
                    self->v[x] /= 2U;
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

        case 0x9:
            if (vx != vy)
                core_increment(self);
            core_increment(self);
            break;

        case 0xA:
            self->i = nnn;
            core_increment(self);
            break;

        case 0xB:
            self->pc = nnn + self->v[0x0];
            break;

        case 0xC:
            self->v[x] = random_byte & kk;
            core_increment(self);
            break;

        case 0xD: {
            self->v[0xF] = 0;

            for (usize line_y = 0; line_y < n; line_y++) {
                const u8 pixel = self->memory[self->i + line_y];

                for (usize line_x = 0; line_x < 8; line_x++) {
                    const u8 msb = 0x80;

                    if ((pixel & (msb >> line_x)) != 0) {
                        const u8 index_x = (vx + (u8)(line_x)) % GRAPHICS_WIDTH;
                        const u8 index_y =
                            (vy + (u8)(line_y)) % GRAPHICS_HEIGHT;

                        const usize index = index_x + index_y * GRAPHICS_WIDTH;

                        if (self->graphics[index] == 0)
                            self->v[0xF] = 1;

                        self->graphics[index] ^= 1U;
                    }
                }
            }

            core_increment(self);
        } break;

        case 0xE:
            if (kk == 0x9E && self->keys[vx] == 1) {
                core_increment(self);
            } else if (kk == 0xA1 && self->keys[vx] != 1) {
                core_increment(self);
            }
            core_increment(self);
            break;

        case 0xF: {
            switch (kk) {
                case 0x07:
                    self->v[x] = self->delay_timer;
                    break;

                case 0x15:
                    self->delay_timer = self->v[x];
                    break;

                case 0x18:
                    self->sound_timer = self->v[x];
                    break;

                case 0x1E:
                    self->i += self->v[x];
                    break;

                case 0x33:
                    self->memory[self->i] = self->v[x] / 100;
                    self->memory[self->i + 1U] = (self->v[x] / 10) % 10;
                    self->memory[self->i + 2U] = self->v[x] % 10;
                    break;

                case 0x55:
                    for (usize i = 0; i <= x; i++) {
                        self->memory[self->i + i] = self->v[i];
                    }
                    break;

                case 0x65:
                    for (usize i = 0; i <= x; i++) {
                        self->v[i] = self->memory[self->i + i];
                    }
                    break;
            }
            core_increment(self);
        } break;

        default:
            goto unknown;
    }

    return true;

unknown:
    ESP_LOGE(TAG, "unknown instruction! (0x%X)", instruction);
    return false;
}

u8* Core_Graphics(CoreHandle self) {
    return self->graphics;
}

void Core_Deinit(CoreHandle self) {
    if (self != NULL) {
        if (self->memory != NULL) {
            free(self->memory);
            self->memory = NULL;
        }
        if (self->graphics != NULL) {
            heap_caps_free(self->graphics);
            self->graphics = NULL;
        }

        core_pool[0].allocated = false;
    }
}
