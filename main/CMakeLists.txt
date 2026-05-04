idf_component_register(
  SRCS
    "main.c"
  INCLUDE_DIRS
    "${CMAKE_SOURCE_DIR}/include"
  REQUIRES
    "core"
)

include("${CMAKE_SOURCE_DIR}/options.cmake")
