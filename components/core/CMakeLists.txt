idf_component_register(
  SRCS
    "core.c"
    "rom.c"
  INCLUDE_DIRS
    "include"
    "${CMAKE_SOURCE_DIR}/include"
)

target_compile_options(${COMPONENT_LIB}
  PRIVATE
    "-fanalyzer"
    "-Wanalyzer-symbol-too-complex"
    "-Wanalyzer-too-complex"
    "-Wall"
    "-Werror"
    "-Wextra"
)
