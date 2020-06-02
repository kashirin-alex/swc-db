#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#

  

ADD_TEST_TARGET(
  NAME      cellstore
  SRCS      test_cellstore.cc
  TARGETS   swcdb_core swcdb_core_config swcdb_core_comm swcdb_core_sys swcdb_fs swcdb ${BUILTIN_FS_TARGETS}
  ARGS      "--swc.fs=local" "--swc.rgr.ram.percent=2" "--swc.cfg.path=${CMAKE_INSTALL_PREFIX}/etc/swcdb/"

  SHARED    ${SNAPPY_LIBRARIES_SHARED}
  STATIC    ${SNAPPY_LIBRARIES_STATIC} 
)

ADD_TEST_TARGET(
  NAME      commitlog
  SRCS      test_commitlog.cc
  TARGETS   swcdb_core swcdb_core_config swcdb_core_comm swcdb_core_sys swcdb_fs swcdb ${BUILTIN_FS_TARGETS}
  ARGS      "--swc.fs=local" "--swc.rgr.ram.percent=2" "--swc.cfg.path=${CMAKE_INSTALL_PREFIX}/etc/swcdb/"

  SHARED    ${SNAPPY_LIBRARIES_SHARED}
  STATIC    ${SNAPPY_LIBRARIES_STATIC} 
)
