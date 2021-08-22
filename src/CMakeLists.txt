#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


if((NOT SWC_BUILD_PKG AND NOT SWC_WITHOUT_THRIFT) OR SWC_BUILD_PKG MATCHES "^lib-thrift")
  add_subdirectory(thrift/swcdb)
endif()

add_subdirectory(cc)

add_subdirectory(c)

if(NOT SWC_BUILD_PKG)
  add_subdirectory(py)
  add_subdirectory(rb)
  add_subdirectory(java)
endif()





if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "env")
  install(DIRECTORY etc/swcdb         DESTINATION etc)
  install(FILES py/sbin/swcdb_cluster DESTINATION sbin
    PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
                OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
  )
endif()


if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "lib-core")
  install(FILES     cc/include/swcdb/Version.h  DESTINATION include/swcdb)
  install(DIRECTORY cc/include/swcdb/core       DESTINATION include/swcdb)
  install(DIRECTORY cc/include/swcdb/common     DESTINATION include/swcdb)
endif()





if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG MATCHES "^lib-fs")

  if(NOT SWC_BUILD_PKG)
    install(DIRECTORY cc/include/swcdb/fs         DESTINATION include/swcdb)

  elseif(SWC_BUILD_PKG STREQUAL "lib-fs")
    install(
      FILES cc/include/swcdb/fs/Callbacks.h
            cc/include/swcdb/fs/Dirent.h
            cc/include/swcdb/fs/FileSystem.h
            cc/include/swcdb/fs/Interface.h
            cc/include/swcdb/fs/Settings.h
            cc/include/swcdb/fs/SmartFd.h
      DESTINATION include/swcdb/fs)

  elseif(SWC_BUILD_PKG STREQUAL "lib-fs-local")
    install(DIRECTORY cc/include/swcdb/fs/Local       DESTINATION include/swcdb/fs)

  elseif(SWC_BUILD_PKG STREQUAL "lib-fs-broker")
    install(DIRECTORY cc/include/swcdb/fs/Broker      DESTINATION include/swcdb/fs)

  elseif(SWC_BUILD_PKG STREQUAL "lib-fs-ceph")
    install(DIRECTORY cc/include/swcdb/fs/Ceph        DESTINATION include/swcdb/fs)

  elseif(SWC_BUILD_PKG STREQUAL "lib-fs-hadoop")
    install(DIRECTORY cc/include/swcdb/fs/Hadoop      DESTINATION include/swcdb/fs)

  elseif(SWC_BUILD_PKG STREQUAL "lib-fs-hadoop-jvm")
    install(DIRECTORY cc/include/swcdb/fs/HadoopJVM   DESTINATION include/swcdb/fs)

  endif()

endif()



if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "lib")
  install(DIRECTORY cc/include/swcdb/db         DESTINATION include/swcdb)
endif()



if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "utils")
  install(DIRECTORY cc/include/swcdb/utils      DESTINATION include/swcdb)
endif()



if(THRIFT_CPP_GEN_PATH AND (NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "lib-thrift"))
  install(DIRECTORY cc/include/swcdb/thrift/client      DESTINATION include/swcdb/thrift)
  install(DIRECTORY cc/include/swcdb/thrift/utils       DESTINATION include/swcdb/thrift)
  install(DIRECTORY cc/include/swcdb/thrift/gen-cpp     DESTINATION include/swcdb/thrift)
endif()



if(THRIFT_C_GEN_PATH AND (NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "lib-thrift-c"))
  install(FILES     c/include/swcdb/version.h           DESTINATION include/swcdb)
  install(DIRECTORY c/include/swcdb/thrift/client       DESTINATION include/swcdb/thrift)
  install(DIRECTORY c/include/swcdb/thrift/gen-c_glib   DESTINATION include/swcdb/thrift)
endif()

