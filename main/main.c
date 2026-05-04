#include <stdbool.h>

#include "freertos/FreeRTOS.h"

#include "core.h"
#include "rom.h"

void app_main() {
    CoreHandle core = Core_Init();

    if (!Core_Load(core, rom_name, rom_data, rom_size))
        return;

    while (true) {
        if (!Core_Cycle(core))
            break;
        vTaskDelay(pdMS_TO_TICKS(16));
    }

    Core_Deinit(core);
}
