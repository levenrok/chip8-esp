set(srcs
  "main.c"
)
set(requires)

idf_component_register(SRCS ${srcs}
  INCLUDE_DIRS "include" "${CMAKE_SOURCE_DIR}/include"
  PRIV_REQUIRES ${requires}
)
