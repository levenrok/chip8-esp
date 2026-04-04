#pragma once

#include <stdbool.h>
#include <string.h>

#include "common.h"

#define MAX_ROM_SIZE 3584

typedef struct {
    const char* name;
    u8* data;
    usize size;
} Rom;

Rom rom_new(const char* name, const u8* data, usize size);
void rom_delete(Rom* self);

static inline void rom_set(Rom* self,
                           const char* name,
                           const u8* data,
                           usize size) {
    self->name = name;
    (void)memcpy(self->data, data, size);
    self->size = size;
}

static inline bool rom_check(const Rom* self) {
    if (self->name == NULL || self->data == NULL || self->size == 0)
        return false;

    return true;
}

static inline void rom_clear(Rom* self) {
    self->name = NULL;
    (void)memset(self->data, 0, self->size);
    self->size = 0;
}
