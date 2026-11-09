set(MMO_SRC
  ${MMO_DIR}/mmo_class.cpp
  ${UTIL_DIR}/table.cpp
  ${UTIL_DIR}/debug.cpp
  ${UTIL_DIR}/ast_visitors/eval_expression.cpp
  ${UTIL_DIR}/ast_visitors/partial_eval_expression.cpp
  ${UTIL_DIR}/ast_visitors/to_micro/convert_to_micro.cpp
  ${UTIL_DIR}/ast_visitors/to_micro/convert_to_micro_statement.cpp
  ${UTIL_DIR}/ast_visitors/to_micro/convert_to_micro_expression.cpp
  ${UTIL_DIR}/type.cpp
)

add_library(mmo-lib STATIC ${MMO_SRC})
target_link_libraries(mmo-lib PUBLIC modelica PRIVATE Boost::boost)

add_executable(mmo ${MMO_DIR}/main.cpp)
target_link_libraries(mmo PRIVATE mmo-lib)
