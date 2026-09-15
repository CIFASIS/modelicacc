set(CAUSALIZE_COMMON_SRC
  ${MMO_DIR}/mmo_class.cpp
  ${UTIL_DIR}/debug.cpp
  ${UTIL_DIR}/table.cpp
  ${UTIL_DIR}/type.cpp
  ${UTIL_DIR}/ast_visitors/all_expressions.cpp
  ${UTIL_DIR}/ast_visitors/replace_expression.cpp
  ${UTIL_DIR}/ast_visitors/eval_expression.cpp
  ${UTIL_DIR}/ast_visitors/partial_eval_expression.cpp
  ${UTIL_DIR}/ast_visitors/ginac_interface.cpp
  ${UTIL_DIR}/ast_visitors/contains_expression.cpp
  ${UTIL_DIR}/ast_visitors/splitfor_visitor.cpp
  ${UTIL_DIR}/ast_visitors/state_variables_finder.cpp
  ${UTIL_DIR}/solve/solve.cpp
)

add_subdirectory(sbg_implementation)
