#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


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
  ./properties_parser_test argOne --i16 1k --i32 64K --i64 1G --boo true --strs=1 -strs 2 -strs 3 --strs 4 -strs=5 --strs=6 --b=1 --is_true --swc.cfg=./properties_parser_test.cfg  --quiet
)

ADD_TEST_TARGET(
  NAME      queue
  SRCS      queue.cc
  TARGETS   swcdb_core
)

ADD_TEST_TARGET(
  NAME      checksum
  SRCS      checksum.cc
  TARGETS   swcdb_core
)

ADD_TEST_TARGET(
  NAME      vector
  SRCS      vector.cc
  TARGETS   swcdb_core
)

ADD_TEST_TARGET(
  NAME      map
  SRCS      map.cc
  TARGETS   swcdb_core
)

