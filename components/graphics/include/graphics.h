#pragma once

#include "esp_lcd_panel_ops.h"

#include "types.h"

#define GRAPHICS_WIDTH 64
#define GRAPHICS_HEIGHT 32

esp_lcd_panel_handle_t graphics_init(void);
void graphics_render(esp_lcd_panel_handle_t handle, const u8* restrict buffer);
