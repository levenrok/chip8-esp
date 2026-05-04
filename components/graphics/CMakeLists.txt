idf_component_register(
  SRCS
    "graphics.c"
  INCLUDE_DIRS
    "include"
    "${CMAKE_SOURCE_DIR}/include"
  REQUIRES
    "esp_driver_i2c"
    "esp_lcd"
)

include("${CMAKE_SOURCE_DIR}/options.cmake")
