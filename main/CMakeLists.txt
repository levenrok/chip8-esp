set(srcs
  "chip-8.c"
)
set(requires
  "nvs_flash"
  "esp_wifi"
)

idf_component_register(SRCS ${srcs}
  INCLUDE_DIRS "include"
  PRIV_REQUIRES ${requires}
)
