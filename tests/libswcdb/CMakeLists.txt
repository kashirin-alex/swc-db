#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>



ADD_TEST_TARGET(
  NAME      cell_key
  SRCS      test_cell_key.cc
  TARGETS   swcdb_core swcdb
)

ADD_TEST_TARGET(
  NAME      cell
  SRCS      test_cell.cc
  TARGETS   swcdb_core swcdb
)

ADD_TEST_TARGET(
  NAME      cell_specs_scan
  SRCS      test_cell_specs_scan.cc
  TARGETS   swcdb_core swcdb
)

ADD_TEST_TARGET(
  NAME      cells_interval
  SRCS      test_cells_interval.cc
  TARGETS   swcdb_core swcdb
)

ADD_TEST_TARGET(
  NAME      cells_mutable
  SRCS      test_cells_mutable.cc
  TARGETS   swcdb_core swcdb_core_config swcdb
)

ADD_TEST_TARGET(
  NAME      key_comparator
  SRCS      test_key_comparator.cc
  TARGETS   swcdb_core swcdb_core_config swcdb
)
ADD_TEST_TARGET(
  NAME      key_comparator_impl_src
  SRCS      test_key_comparator.cc
            ${SWC_VERSION_SRC}
  SHARED    ${SWC_LIB_CORE_SHARED}
  STATIC    ${SWC_LIB_CORE_STATIC}
  FLAGS     "-DSWC_IMPL_SOURCE" ${SWC_DEFAULT_PATH_FLAGS}
)

