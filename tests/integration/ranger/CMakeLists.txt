#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>





set(FLAGS )
set(TARGETS )

set(SHARED_TARGETS
  "swcdb_core;swcdb_core_config;swcdb_core_comm;swcdb_fs;swcdb;${BUILTIN_FS_TARGETS}")

if(SWC_IMPL_SOURCE)
  set(FLAGS "-DSWC_IMPL_SOURCE" ${SWC_DEFAULT_PATH_FLAGS})
else()
  set(TARGETS ${SHARED_TARGETS})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${BUILTIN_FS_FLAGS}")



ADD_TEST_TARGET(
  NAME      cellstore
  SRCS      test_cellstore.cc
            ${SWC_VERSION_SRC}
  TARGETS   ${TARGETS}
  ARGS      "--swc.fs=local"
            "--swc.rgr.ram.allowed.percent=2"
            "--swc.cfg.path=${CMAKE_INSTALL_PREFIX}/etc/swcdb/"
            "--swc.fs.lib.local=${CMAKE_INSTALL_PREFIX}/lib/libswcdb_fs_local.so"
            "--swc.rgr.metrics.enabled=false"

  SHARED    ${SWC_LIB_CORE_SHARED}
  STATIC    ${SWC_LIB_CORE_STATIC}
  FLAGS     ${FLAGS}
)

ADD_TEST_TARGET(
  NAME      commitlog
  SRCS      test_commitlog.cc
            ${SWC_VERSION_SRC}
  TARGETS   ${TARGETS}
  ARGS      "--swc.fs=local"
            "--swc.rgr.ram.allowed.percent=2"
            "--swc.cfg.path=${CMAKE_INSTALL_PREFIX}/etc/swcdb/"
            "--swc.fs.lib.local=${CMAKE_INSTALL_PREFIX}/lib/libswcdb_fs_local.so"
            "--swc.rgr.metrics.enabled=false"

  SHARED    ${SWC_LIB_CORE_SHARED}
  STATIC    ${SWC_LIB_CORE_STATIC}
  FLAGS     ${FLAGS}
)
