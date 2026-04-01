#pragma once

#include "esp_http_server.h"

httpd_handle_t server_start();

static inline void server_stop(httpd_handle_t* server) {
    if (*server != NULL) {
        (void)httpd_stop(*server);
        *server = NULL;
    }
}
