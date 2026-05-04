#include <string.h>

#include "driver/i2c_master.h"
#include "sdkconfig.h"

#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"

#include "include/graphics.h"

#define I2C_BUS_PORT 0

esp_lcd_panel_handle_t graphics_init(void) {
    i2c_master_bus_handle_t bus_handle = NULL;
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_BUS_PORT,
        .sda_io_num = CONFIG_I2C_PIN_SDA,
        .scl_io_num = CONFIG_I2C_PIN_SCL,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = CONFIG_I2C_HW_ADDRESS,
        .scl_speed_hz = CONFIG_I2C_CLK_SPEED_HZ,
        .dc_bit_offset = 6,
        .control_phase_bytes = 1,
        .lcd_cmd_bits = CONFIG_DISPLAY_CMD_BITS,
        .lcd_param_bits = CONFIG_DISPLAY_PARAM_BITS,
    };

    ESP_ERROR_CHECK(
        esp_lcd_new_panel_io_i2c(bus_handle, &io_config, &io_handle));

    esp_lcd_panel_ssd1306_config_t vendor_config = {
        .height = CONFIG_DISPLAY_V_RES,
    };

    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .reset_gpio_num = -1,
        .vendor_config = &vendor_config,
    };

    ESP_ERROR_CHECK(
        esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    return panel_handle;
}

void graphics_render(esp_lcd_panel_handle_t handle, u8* buffer) {
    static u8 packed_buffer[GRAPHICS_WIDTH * GRAPHICS_HEIGHT / 8];
    (void)memset(packed_buffer, 0, sizeof(packed_buffer));

    for (usize y = 0; y < GRAPHICS_HEIGHT; y++) {
        for (usize x = 0; x < GRAPHICS_WIDTH; x++) {
            const usize offset = x + y * GRAPHICS_WIDTH;

            if (buffer[offset] != 0) {
                packed_buffer[x + (y / 8) * GRAPHICS_WIDTH] |= (1U << (y % 8));
            }
        }
    }

    // Center the bitmap if canvas is smaller than display, otherwise start at 0
    const i32 x_start = CONFIG_DISPLAY_H_RES != GRAPHICS_WIDTH
                            ? (CONFIG_DISPLAY_H_RES - GRAPHICS_WIDTH) / 2
                            : 0;
    const i32 y_start = CONFIG_DISPLAY_V_RES != GRAPHICS_HEIGHT
                            ? (CONFIG_DISPLAY_V_RES - GRAPHICS_HEIGHT) / 2
                            : 0;

    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(
        handle, x_start, y_start, x_start + GRAPHICS_WIDTH,
        y_start + GRAPHICS_HEIGHT, packed_buffer));
}
