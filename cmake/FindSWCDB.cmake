#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>



if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "lib-core")
  SET(SWC_LIB_CORE_TARGET         swcdb_core)
  SET(SWC_LIB_CORE_CONFIG_TARGET  swcdb_core_config)
  SET(SWC_LIB_CORE_COMM_TARGET    swcdb_core_comm)
else ()
  SET_DEPS(NAME "SWC_LIB_CORE"        REQUIRED TRUE   STATIC libswcdb_core.a        SHARED swcdb_core         INCLUDE swcdb/core/Compat.h)
  SET_DEPS(NAME "SWC_LIB_CORE_CONFIG" REQUIRED TRUE   STATIC libswcdb_core_config.a SHARED swcdb_core_config)

  if(NOT SWC_BUILD_PKG OR NOT SWC_BUILD_PKG MATCHES "^lib-fs" OR SWC_BUILD_PKG STREQUAL "lib-fs-broker")
    SET_DEPS(NAME "SWC_LIB_CORE_COMM"   REQUIRED TRUE   STATIC libswcdb_core_comm.a   SHARED swcdb_core_comm)
  endif()
  SET(SWC_LIB_CORE_TARGET        )
  SET(SWC_LIB_CORE_CONFIG_TARGET )
  SET(SWC_LIB_CORE_COMM_TARGET   )
endif()




if(NOT SWC_BUILD_PKG STREQUAL "lib-core")

  if(NOT SWC_BUILD_PKG MATCHES "^lib-fs") # AND NOT SWC_BUILD_PKG STREQUAL "fsbroker"
    if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "lib")
      SET(SWC_LIB_TARGET swcdb)
    else()
      SET_DEPS(NAME "SWC_LIB"  REQUIRED TRUE STATIC libswcdb.a     SHARED swcdb)
      SET(SWC_LIB_TARGET )
    endif()
  endif()


  if(NOT SWC_BUILD_PKG STREQUAL "lib")
    if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "lib-fs")
      SET(SWC_LIB_FS_TARGET  swcdb_fs)
    else()
      SET_DEPS(NAME "SWC_LIB_FS" REQUIRED TRUE STATIC libswcdb_fs.a  SHARED swcdb_fs)
      SET(SWC_LIB_FS_TARGET )
    endif()
  endif()

endif()
