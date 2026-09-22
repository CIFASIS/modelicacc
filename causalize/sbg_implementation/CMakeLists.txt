set(CAUSALIZE_SBG_DIR ${CMAKE_CURRENT_LIST_DIR})

set(CAUSALIZE_SBG_SRC
  main.cpp
  ${CAUSALIZE_SBG_DIR}/algebraic_loops.cpp
  ${CAUSALIZE_SBG_DIR}/algebraic_loops_detection.cpp
  ${CAUSALIZE_SBG_DIR}/causalize.cpp
  ${CAUSALIZE_SBG_DIR}/causal_model.cpp
  ${CAUSALIZE_SBG_DIR}/generate_sbg_input.cpp
  ${CAUSALIZE_SBG_DIR}/horizontal_sorting.cpp
  ${CAUSALIZE_SBG_DIR}/model_match.cpp
  ${CAUSALIZE_SBG_DIR}/tearing.cpp
  ${CAUSALIZE_SBG_DIR}/vertical_sorting.cpp
  ${CAUSALIZE_SBG_DIR}/tearing_variables.cpp
  ${CAUSALIZE_SBG_DIR}/ast_visitors/equation_info_visitor.cpp
  ${CAUSALIZE_SBG_DIR}/ast_visitors/eq_var_match_annotator.cpp
  ${CAUSALIZE_SBG_DIR}/ast_visitors/residual_equation.cpp
  ${CAUSALIZE_SBG_DIR}/ast_visitors/variable_renamer.cpp
  ${CAUSALIZE_SBG_DIR}/builders/causalization_builders.cpp
  ${CAUSALIZE_SBG_DIR}/builders/causalization_json.cpp
  ${UTIL_DIR}/logger.cpp
  ${UTIL_DIR}/ast_visitors/constant_expression.cpp
  ${UTIL_DIR}/ast_visitors/eval_integer.cpp
  ${UTIL_DIR}/ast_visitors/flatter_for.cpp
  ${UTIL_DIR}/ast_visitors/is_integer.cpp
  ${UTIL_DIR}/ast_visitors/matching_exps.cpp
  ${UTIL_DIR}/ast_visitors/replace_equation.cpp
  ${UTIL_DIR}/sbg/conversions.cpp
  ${UTIL_DIR}/sbg/equation_info.cpp
  ${UTIL_DIR}/sbg/modelica_sbg.cpp
  ${UTIL_DIR}/sbg/set_vertex.cpp
  ${UTIL_DIR}/sbg/set_edge.cpp
  ${UTIL_DIR}/sbg/ast_visitors/access_to_sbg_expr.cpp
  ${UTIL_DIR}/sbg/ast_visitors/equation_sbg_set.cpp
  ${UTIL_DIR}/sbg/ast_visitors/sbg_expr_visitor.cpp
  ${UTIL_DIR}/sbg/ast_visitors/sbg_set_visitor.cpp
)

add_executable(causalize
  ${CAUSALIZE_SBG_SRC}
  ${CAUSALIZE_COMMON_SRC}
)

target_include_directories(causalize PRIVATE ${BOOST_1_81_DIR}/include)
target_link_libraries(causalize PRIVATE modelica ginac sbgraph ModelicaCC::profiler)

add_custom_target(causalize_test
  COMMAND ${CMAKE_COMMAND} -E echo "BUILDING CAUSALIZE TESTS: not implemented"
  COMMAND ${CMAKE_COMMAND} -E echo "RUNNING CAUSALIZE TESTS: not implemented"
)
