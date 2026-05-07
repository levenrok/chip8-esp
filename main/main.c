#include <stdbool.h>

#include "freertos/FreeRTOS.h"

#include "esp_lcd_types.h"

#include "core.h"
#include "graphics.h"
#include "rom.h"

void app_main() {
    CoreHandle core = Core_Init();

    if (!Core_Load(core, rom_name, rom_data, rom_size))
        return;

    esp_lcd_panel_handle_t panel = graphics_init();

    while (true) {
        if (!Core_Cycle(core))
            break;
        graphics_render(panel, Core_Graphics(core));

        vTaskDelay(pdMS_TO_TICKS(16));
    }

    Core_Deinit(core);
}
