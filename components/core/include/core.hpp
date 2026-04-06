#pragma once

#include "common.h"

constexpr usize MEMORY_SIZE = 4096;
constexpr usize GRAPHICS_SIZE = 64 * 32;

constexpr usize MAX_ROM_SIZE = 4096 - 512;

class Core {
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

    void increment(void) { sp += 2; }

  public:
    Core(void);
    ~Core(void);

    [[nodiscard("must check if the core initialized successfully")]]
    bool check(Core* const core);

    [[nodiscard("must check if the rom loaded successfully")]]
    bool load(const char* name, const u8* data, usize size);

    void cycle(void);
};
