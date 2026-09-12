set(ANTIALIAS_SRC
  main.cpp
  ${ANTIALIAS_DIR}/remove_alias.cpp
  ${MMO_DIR}/mmo_class.cpp
  ${UTIL_DIR}/ast_visitors/eval_expression.cpp
  ${UTIL_DIR}/ast_visitors/partial_eval_expression.cpp
  ${UTIL_DIR}/ast_visitors/replace_expression.cpp
  ${UTIL_DIR}/ast_visitors/replace_equation.cpp
  ${UTIL_DIR}/ast_visitors/replace_statement.cpp
  ${UTIL_DIR}/debug.cpp
  ${UTIL_DIR}/table.cpp
  ${UTIL_DIR}/type.cpp
  ${UTIL_DIR}/ast_visitors/state_variables_finder.cpp
)

add_executable(antialias ${ANTIALIAS_SRC})
target_link_libraries(antialias PRIVATE modelica ginac)
