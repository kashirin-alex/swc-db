#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>



add_test(
  NAME    test-swcdb_cluster-start_managers
  COMMAND ${CMAKE_INSTALL_PREFIX}/sbin/swcdb_cluster start-managers
)


ADD_TEST_TARGET(
  NAME      test_echo
  SRCS      test_echo.cc
  TARGETS   swcdb_core swcdb_core_config swcdb_core_comm swcdb
  ARGS      "--swc.cfg.path=${CMAKE_INSTALL_PREFIX}/etc/swcdb/"
)


add_test(
  NAME    test-swcdb_cluster-stop_managers
  COMMAND ${CMAKE_INSTALL_PREFIX}/sbin/swcdb_cluster stop-managers
)