#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"

#include "common.h"
#include "include/server.h"

static const char* TAG = "server";
static esp_err_t root_uri_handler(httpd_req_t* req) {
    return httpd_resp_sendstr(req, "Hello, World!");
}

static const httpd_uri_t root_uri = {
    .method = HTTP_GET,
    .uri = "/",
    .handler = root_uri_handler,
    .user_ctx = NULL,
};

httpd_handle_t server_start() {
    httpd_handle_t server = NULL;
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server, &cfg) == ESP_OK) {
        ESP_LOGI(TAG, "server started.");
        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &root_uri));

        return server;
    }

    return NULL;
}

inline void server_stop(httpd_handle_t* server) {
    if (*server != NULL) {
        (void)httpd_stop(*server);
        *server = NULL;
    }
}
