set(srcs
  "main.c"
  "server.c"
)
set(requires
  "nvs_flash"
  "esp_http_server"
  "esp_wifi"
)

idf_component_register(SRCS ${srcs}
  INCLUDE_DIRS "include" "${CMAKE_SOURCE_DIR}/include"
  PRIV_REQUIRES ${requires}
)
