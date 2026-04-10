idf_component_register(
  SRCS
    "core.cpp"
  INCLUDE_DIRS
    "include"
    "${CMAKE_SOURCE_DIR}/include"
)

include("${CMAKE_SOURCE_DIR}/options.cmake")
