#include "stdlib.h"

#include "include/rom.h"

Rom rom_new(const char* name, const u8* data, usize size) {
    if (size > MAX_ROM_SIZE)
        goto err;

    Rom rom = {
        .name = name,
        .data = calloc(MAX_ROM_SIZE, sizeof(u8)),
        .size = size,
    };

    if (rom.data == NULL)
        goto err;

    (void)memcpy(rom.data, data, size);

    return rom;

err:
    return (Rom){0};
}

void rom_delete(Rom* self) {
    if (self->data != NULL) {
        free(self->data);

        self->name = NULL;
        self->data = NULL;
        self->size = 0;
    }
}
