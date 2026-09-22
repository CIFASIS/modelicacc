set(FLATTER_SRC
  ${FLATTER_DIR}/main.cpp
  ${FLATTER_DIR}/class_finder.cpp
  ${FLATTER_DIR}/connectors.cpp
  ${FLATTER_DIR}/flatter.cpp
  ${FLATTER_DIR}/generate_sbg_file.cpp
  ${FLATTER_DIR}/remove_composition.cpp
  ${MMO_DIR}/mmo_class.cpp
  ${MMO_DIR}/mmo_tree.cpp
  ${UTIL_DIR}/debug.cpp
  ${UTIL_DIR}/logger.cpp
  ${UTIL_DIR}/table.cpp
  ${UTIL_DIR}/type.cpp
  ${UTIL_DIR}/ast_visitors/constant_expression.cpp
  ${UTIL_DIR}/ast_visitors/contains_expression.cpp
  ${UTIL_DIR}/ast_visitors/contains_expression_flatter.cpp
  ${UTIL_DIR}/ast_visitors/dot_expression.cpp
  ${UTIL_DIR}/ast_visitors/eval_expression.cpp
  ${UTIL_DIR}/ast_visitors/eval_integer.cpp
  ${UTIL_DIR}/ast_visitors/mark_connector.cpp
  ${UTIL_DIR}/ast_visitors/matching_exps.cpp
  ${UTIL_DIR}/ast_visitors/partial_eval_expression.cpp
  ${UTIL_DIR}/ast_visitors/replace_expression.cpp
  ${UTIL_DIR}/sbg/conversions.cpp
  ${UTIL_DIR}/sbg/equation_info.cpp
  ${UTIL_DIR}/sbg/modelica_sbg.cpp
  ${UTIL_DIR}/sbg/set_edge.cpp
  ${UTIL_DIR}/sbg/set_vertex.cpp
  ${UTIL_DIR}/sbg/ast_visitors/access_to_sbg_expr.cpp
  ${UTIL_DIR}/sbg/ast_visitors/sbg_expr_visitor.cpp
)

add_executable(flatter ${FLATTER_SRC})

target_include_directories(flatter PRIVATE ${BOOST_1_81_DIR}/include)
target_link_libraries(flatter PRIVATE modelica ginac sbgraph ModelicaCC::profiler)

add_subdirectory(test)
