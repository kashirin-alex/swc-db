#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#




ADD_TEST_TARGET(
  NAME      test_echo
  SRCS      test_echo.cc
  TARGETS   swcdb_core swcdb_core_config swcdb_core_comm swcdb
  ARGS      "--swc.cfg.path=${CMAKE_INSTALL_PREFIX}/etc/swcdb/"
)