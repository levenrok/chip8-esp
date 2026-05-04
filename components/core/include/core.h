#pragma once

#include <stdbool.h>

#include "graphics.h"
#include "types.h"

#define MAX_ROM_SIZE 4096 - 512

#define MEMORY_SIZE 4096
#define GRAPHICS_SIZE (GRAPHICS_WIDTH * GRAPHICS_HEIGHT)

typedef struct Core* CoreHandle;

CoreHandle Core_Init(void);
bool Core_Load(CoreHandle self, const char* name, const u8* data, usize size);
bool Core_Cycle(CoreHandle self);
void Core_Deinit(CoreHandle self);
