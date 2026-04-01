set(srcs
  "chip-8.c"
  "server.c"
)
set(requires
  "nvs_flash"
  "esp_http_server"
  "esp_wifi"
)

idf_component_register(SRCS ${srcs}
  INCLUDE_DIRS "include"
  PRIV_REQUIRES ${requires}
)
