set(FLATTER_SRC
  ${FLATTER_DIR}/main.cpp
  ${FLATTER_DIR}/class_finder.cpp
  ${FLATTER_DIR}/connectors.cpp
  ${FLATTER_DIR}/flatter.cpp
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
  ${UTIL_DIR}/ast_visitors/partial_eval_expression.cpp
  ${UTIL_DIR}/ast_visitors/replace_expression.cpp
  ${UTIL_DIR}/sbg/conversions.cpp
)

add_executable(flatter ${FLATTER_SRC})

target_include_directories(flatter PRIVATE ${BOOST_1_81_DIR}/include)
add_dependencies(flatter lib-boost)
target_link_libraries(flatter PRIVATE modelica ginac SBG::dev)

add_subdirectory(test)
