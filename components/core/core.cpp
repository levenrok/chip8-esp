#include <cstring>

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_random.h"

#include "include/core.hpp"

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

Core::Core(void) {
    ESP_LOGI(TAG, "initializing emulator core...");

    memory = static_cast<u8*>(
        heap_caps_calloc(MEMORY_SIZE, sizeof(u8), MALLOC_CAP_DEFAULT));
    graphics = static_cast<u8*>(
        heap_caps_calloc(GRAPHICS_SIZE, sizeof(u8), MALLOC_CAP_DMA));

    (void)memset(v, 0, sizeof(v));
    (void)memset(stack, 0, sizeof(stack));
    (void)memset(keys, 0, sizeof(keys));

    i = 0;
    opcode = 0;
    pc = 0;
    sp = 0;
    delay_timer = 0;
    sound_timer = 0;
}

Core::~Core(void) {
    if (memory != NULL) {
        heap_caps_free(memory);
        memory = NULL;
    } else if (graphics != NULL) {
        heap_caps_free(graphics);
        graphics = NULL;
    }
}

bool Core::check(void) {
    if (memory == NULL || graphics == NULL) {
        ESP_LOGE(TAG, "failed to initialize the core!");
        return false;
    }

    ESP_LOGI(TAG, "core initialized.");
    return true;
}

bool Core::load(const char* name, const u8* data, usize size) {
    ESP_LOGI(TAG, "loading ROM: name: %s, size: %d", name, size);

    if (size > MAX_ROM_SIZE) {
        ESP_LOGE(TAG, "failed to load ROM! (max: %d bytes, current: %d bytes)",
                 MAX_ROM_SIZE, size);
        return false;
    }

    (void)memcpy(memory, FONT_SET, sizeof(FONT_SET));
    (void)memcpy(memory + 0x0200, data, size);

    ESP_LOGI(TAG, "ROM loaded.");
    return true;
}

void Core::cycle(void) {
    opcode = memory[pc] << 8 | memory[pc + 1];

    const u8 random_byte = esp_random() % 255;

    const u8 instruction = (opcode & 0xF000) >> 12;
    const u16 nnn = opcode & 0x0FFF;
    const u8 n = opcode & 0x000F;
    const u8 x = (opcode & 0x0F00) >> 8;
    const u8 y = (opcode & 0x00F0) >> 4;
    const u8 kk = opcode & 0x00FF;

    const u8 vx = v[x];
    const u8 vy = v[y];

    ESP_LOGI(TAG, "instruction: 0x%02X, opcode: 0x%04X, counter: 0x%04X",
             instruction, opcode, pc);

    switch (instruction) {
        case 0x0:
            if (kk == 0xE0) {
                (void)memset(graphics, 0, GRAPHICS_SIZE);
            } else if (kk == 0xEE) {
                pc = stack[sp];
                sp -= 1;
            }
            increment();
            break;

        case 0x1:
            pc = nnn;
            break;

        case 0x2:
            sp += 1;
            stack[sp] = pc;
            pc = nnn;
            break;

        case 0x3:
            if (vx == kk)
                increment();
            increment();
            break;

        case 0x4:
            if (vx != kk)
                increment();
            increment();
            break;

        case 0x5:
            if (vx == vy)
                increment();
            increment();
            break;

        case 0x6:
            v[x] = kk;
            increment();
            break;

        case 0x7:
            v[x] = vx + kk;
            increment();
            break;

        case 0x8: {
            switch (n) {
                case 0x0:
                    v[x] = vy;
                    break;

                case 0x1:
                    v[x] |= vy;
                    break;

                case 0x2:
                    v[x] &= vy;
                    break;

                case 0x3:
                    v[x] ^= vy;
                    break;

                case 0x4: {
                    u8 result = 0;
                    bool overflowing = __builtin_add_overflow(vx, vy, &result);

                    v[0xF] = overflowing ? 1 : 0;
                    v[x] = result;
                } break;

                case 0x5:
                    v[0xF] = vx > vy ? 1 : 0;
                    v[x] -= vy;
                    break;

                case 0x6:
                    v[0xF] = vx << 1 == 1 ? 1 : 0;
                    v[x] /= 2;
                    break;

                case 0x7:
                    v[0xF] = vy > vx ? 1 : 0;
                    v[x] = vy - vx;
                    break;

                case 0xE:
                    v[0xF] = vx >> 1 == 1 ? 1 : 0;
                    v[x] *= 2;
                    break;

                default:
                    goto unknown;
            }
            increment();
        } break;

        case 0x9:
            if (vx != vy)
                increment();
            increment();
            break;

        case 0xA:
            i = nnn;
            increment();
            break;

        case 0xB:
            pc = nnn + v[0x0];
            break;

        case 0xC:
            v[x] = random_byte & kk;
            increment();
            break;

        case 0xD: {
            v[0xF] = 0;

            for (usize line_y = 0; line_y < n; line_y++) {
                const u8 pixel = memory[i + line_y];

                for (usize line_x = 0; line_x < 8; line_x++) {
                    const u8 msb = 0x80;

                    if ((pixel & (msb >> line_x)) != 0) {
                        const u8 index_x = (vx + static_cast<u8>(line_x)) % 64;
                        const u8 index_y = (vy + static_cast<u8>(line_y)) % 32;

                        const u8 index = index_x + index_y * 64;

                        if (graphics[index] == 0)
                            v[0xF] = 1;

                        graphics[index] ^= 1;
                    }
                }
            }

            increment();
        } break;

        case 0xE:
            if (kk == 0x9E && keys[vx] == 1) {
                increment();
            } else if (kk == 0xA1 && keys[vx] != 1) {
                increment();
            }
            increment();
            break;

        default:
            goto unknown;
    }

    return;

unknown:
    ESP_LOGE(TAG, "unknown instruction! (0x%X)", instruction);
    return;
}
