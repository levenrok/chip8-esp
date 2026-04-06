idf_component_register(
  SRCS
    "core.cpp"
  INCLUDE_DIRS
    "include"
    "${CMAKE_SOURCE_DIR}/include"
)

target_compile_options(${COMPONENT_LIB}
  PRIVATE
    "-fanalyzer"
    "-Wall"
    "-Werror"
    "-Wextra"
)
