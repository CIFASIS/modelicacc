set(CAUSALIZE_SBG_DIR ${CMAKE_CURRENT_LIST_DIR})

set(CAUSALIZE_SBG_SRC
  main.cpp
  ${CAUSALIZE_SBG_DIR}/algebraic_loops.cpp
  ${CAUSALIZE_SBG_DIR}/algebraic_loops_detection.cpp
  ${CAUSALIZE_SBG_DIR}/causalize.cpp
  ${CAUSALIZE_SBG_DIR}/causal_model.cpp
  ${CAUSALIZE_SBG_DIR}/equation_info.cpp
  ${CAUSALIZE_SBG_DIR}/generate_sbg_input.cpp
  ${CAUSALIZE_SBG_DIR}/horizontal_sorting.cpp
  ${CAUSALIZE_SBG_DIR}/modelica_sbg.cpp
  ${CAUSALIZE_SBG_DIR}/model_match.cpp
  ${CAUSALIZE_SBG_DIR}/set_vertex.cpp
  ${CAUSALIZE_SBG_DIR}/set_edge.cpp
  ${CAUSALIZE_SBG_DIR}/tearing.cpp
  ${CAUSALIZE_SBG_DIR}/vertical_sorting.cpp
  ${CAUSALIZE_SBG_DIR}/tearing_variables.cpp
  ${CAUSALIZE_SBG_DIR}/ast_visitors/eq_var_match_annotator.cpp
  ${CAUSALIZE_SBG_DIR}/ast_visitors/residual_equation.cpp
  ${CAUSALIZE_SBG_DIR}/ast_visitors/variable_renamer.cpp
  ${CAUSALIZE_SBG_DIR}/builders/causalization_builders.cpp
  ${CAUSALIZE_SBG_DIR}/builders/causalization_json.cpp
  ${UTIL_DIR}/logger.cpp
  ${UTIL_DIR}/sbg_interface.cpp
  ${UTIL_DIR}/ast_visitors/constant_expression.cpp
  ${UTIL_DIR}/ast_visitors/equation_sbg_set.cpp
  ${UTIL_DIR}/ast_visitors/eval_integer.cpp
  ${UTIL_DIR}/ast_visitors/is_integer.cpp
  ${UTIL_DIR}/ast_visitors/matching_exps.cpp
  ${UTIL_DIR}/ast_visitors/replace_equation.cpp
  ${UTIL_DIR}/ast_visitors/sbg_expr_visitor.cpp
  ${UTIL_DIR}/ast_visitors/sbg_set_visitor.cpp
)

add_executable(causalize
  ${CAUSALIZE_SBG_SRC}
  ${CAUSALIZE_COMMON_SRC}
)

target_include_directories(causalize PRIVATE ${BOOST_1_81_DIR}/include)
add_dependencies(causalize lib-boost)
target_link_libraries(causalize PRIVATE modelica ginac SBG::dev)

add_custom_target(causalize_test
  COMMAND ${CMAKE_COMMAND} -E echo "BUILDING CAUSALIZE TESTS: not implemented"
  COMMAND ${CMAKE_COMMAND} -E echo "RUNNING CAUSALIZE TESTS: not implemented"
)
