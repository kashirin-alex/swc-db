#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


add_test(
  NAME    test-swcdb_cluster-start
  COMMAND ${CMAKE_INSTALL_PREFIX}/sbin/swcdb_cluster start
)

ADD_TEST_TARGET(
  NAME      bkr_mng_column
  SRCS      test_bkr_mng_column.cc
  TARGETS   swcdb_core swcdb_core_config swcdb_core_comm swcdb
  ARGS      "--swc.cfg.path=${CMAKE_INSTALL_PREFIX}/etc/swcdb/"
)

add_test(
  NAME    test-swcdb_cluster-stop
  COMMAND ${CMAKE_INSTALL_PREFIX}/sbin/swcdb_cluster stop
)
