#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#


ADD_TEST_TARGET(
  NAME    core-serialization
  SRCS    sertest.cc
  TARGETS swcdb_core 
)

ADD_TEST_TARGET(
  NAME      comparators
  SRCS      test_comparators.cc
  TARGETS   swcdb_core
)

ADD_TEST_TARGET(
  NAME      page-arena
  SRCS      pagearena.cc
  TARGETS   swcdb_core 
)

ADD_TEST_EXEC(
  NAME      properties_parser_test
  SRCS      properties_parser_test.cc
  TARGETS   swcdb_core swcdb_core_config 
)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/properties_parser_test.cfg ${CMAKE_CURRENT_BINARY_DIR}/properties_parser_test.cfg)
add_test(properties_parser_test ${TEST_DIFF}
  ${CMAKE_CURRENT_SOURCE_DIR}/properties_parser_test.golden
  ./properties_parser_test argOne --i16 1k --i32 64K --i64 1G --boo true --strs=1 -strs 2 -strs 3 --strs 4 -strs=5 --strs=6 --b=1 --is_true --swc.cfg=./properties_parser_test.cfg
)

ADD_TEST_TARGET(
  NAME      queue-safe
  SRCS      queuesafe.cc
  TARGETS   swcdb_core 
)


