idf_component_register(
  SRCS
    "main.cpp"
  INCLUDE_DIRS
    "include"
    "${CMAKE_SOURCE_DIR}/include"
  REQUIRES core
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
