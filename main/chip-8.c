#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#include "esp_check.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"

#include "include/common.h"
#include "include/server.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static const char* WIFI_TAG = "wifi";
static const char* SERVER_TAG = "server";

static EventGroupHandle_t wifi_event_group;

static void wifi_event_handler(void* arg,
                               esp_event_base_t event_base,
                               i32 event_id,
                               void* event_data);

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    (void)esp_netif_create_default_wifi_sta();

    const wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_cfg));

    esp_event_handler_instance_t any_id_instance;
    esp_event_handler_instance_t got_ip_instance;
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler,
                               &any_id_instance);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                               wifi_event_handler, &got_ip_instance);

    wifi_config_t cfg = {
        .sta =
            {
                .ssid = CONFIG_SERVER_WIFI_SSID,
                .password = CONFIG_SERVER_WIFI_PASS,
                .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t wifi_event_bits = xEventGroupWaitBits(
        wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE,
        portMAX_DELAY);
    ESP_RETURN_VOID_ON_FALSE(wifi_event_bits & WIFI_CONNECTED_BIT, WIFI_TAG,
                             "failed to connect to wi-fi!");
    ESP_LOGI(WIFI_TAG, "connected to wi-fi.");

    httpd_handle_t server = server_start();
    ESP_RETURN_VOID_ON_FALSE(server != NULL, SERVER_TAG,
                             "failed to start the server!");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    server_stop(&server);
}

static void wifi_event_handler(void* arg,
                               esp_event_base_t event_base,
                               i32 event_id,
                               void* event_data) {
    static i32 wifi_retry_count = 0;

    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START) {
            ESP_LOGI(WIFI_TAG, "connecting to wi-fi...");
            esp_wifi_connect();
        } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            ESP_LOGW(WIFI_TAG, "wi-fi disconnected!");
            if (wifi_retry_count < CONFIG_SERVER_WIFI_MAXIMUM_RETRY) {
                wifi_retry_count += 1;
                ESP_LOGI(WIFI_TAG, "attempting to reconnect... (attempt %d)",
                         wifi_retry_count);
                esp_wifi_connect();
            } else {
                xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
            }
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        const ip_event_got_ip_t* ip_data = (ip_event_got_ip_t*)event_data;
        ESP_LOGI(WIFI_TAG, "info => ip: " IPSTR ", gateway: " IPSTR,
                 IP2STR(&ip_data->ip_info.ip), IP2STR(&ip_data->ip_info.gw));

        wifi_retry_count = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}
