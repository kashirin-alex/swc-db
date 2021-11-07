#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


set(FILE_SYSTEMS "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${BUILTIN_FS_FLAGS}")
set(SWC_LIB_FS_SHARED )
set(SWC_LIB_FS_STATIC )



if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "lib-fs")

########### FILESYSTEM CORE/BASE ###########

set(swcdb_fscore_sources
  ${SWC_VERSION_SRC}
  Statistics.cc
  Dirent.cc
  SmartFd.cc
  Settings.cc
  FileSystem.cc
  Interface.cc
)
ADD_LIB_TARGET(
  NAME      swcdb_fs
  SRCS      ${swcdb_fscore_sources}
  TARGETS   ${SWC_LIB_CORE_TARGET}
            ${SWC_LIB_CORE_CONFIG_TARGET}
            ${BUILTIN_FS_TARGETS}
  SHARED    ${SWC_LIB_CORE_LIBRARIES_SHARED}
            ${SWC_LIB_CORE_CONFIG_LIBRARIES_SHARED}
  STATIC    ${SWC_LIB_CORE_LIBRARIES_STATIC}
            ${SWC_LIB_CORE_CONFIG_LIBRARIES_STATIC}
)
#########################

endif()





if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "lib-fs-broker")

########### FSBROKER FILESYSTEM ###########

set(FILE_SYSTEMS ${FILE_SYSTEMS} broker)
set(swcdb_fsBroker_sources
  ${SWC_VERSION_SRC}
  Broker/FileSystem.cc
  Broker/Protocol/Commands.cc
  Broker/Protocol/params/Append.cc
  Broker/Protocol/params/Close.cc
  Broker/Protocol/params/Create.cc
  Broker/Protocol/params/Exists.cc
  Broker/Protocol/params/Flush.cc
  Broker/Protocol/params/Length.cc
  Broker/Protocol/params/Mkdirs.cc
  Broker/Protocol/params/Open.cc
  Broker/Protocol/params/Pread.cc
  Broker/Protocol/params/Read.cc
  Broker/Protocol/params/ReadAll.cc
  Broker/Protocol/params/Readdir.cc
  Broker/Protocol/params/Remove.cc
  Broker/Protocol/params/Rename.cc
  Broker/Protocol/params/Rmdir.cc
  Broker/Protocol/params/Seek.cc
  Broker/Protocol/params/Sync.cc
  Broker/Protocol/params/Write.cc
  Broker/Protocol/params/CombiPread.cc

  Broker/Protocol/req/Base.cc
)
set(cross_target)
if(NOT SWC_BUILD_PKG AND NOT swcdb_fs_broker IN_LIST BUILTIN_FS_TARGETS)
  set(cross_target swcdb_fs)
endif()

ADD_LIB_TARGET(
  NAME      swcdb_fs_broker
  SRCS      ${swcdb_fsBroker_sources}
  TARGETS   ${SWC_LIB_CORE_TARGET}
            ${SWC_LIB_CORE_CONFIG_TARGET}
            ${SWC_LIB_CORE_COMM_TARGET}
            ${cross_target}
  SHARED    ${SWC_LIB_CORE_LIBRARIES_SHARED}
            ${SWC_LIB_CORE_CONFIG_LIBRARIES_SHARED}
            ${SWC_LIB_CORE_COMM_LIBRARIES_SHARED}
            ${SWC_LIB_FS_LIBRARIES_SHARED}
  STATIC    ${SWC_LIB_CORE_LIBRARIES_STATIC}
            ${SWC_LIB_CORE_CONFIG_LIBRARIES_STATIC}
            ${SWC_LIB_CORE_COMM_LIBRARIES_STATIC}
            ${SWC_LIB_FS_LIBRARIES_STATIC}
)
#########################

endif()



if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "lib-fs-local")

########### LOCAL FILESYSTEM ###########

set(FILE_SYSTEMS ${FILE_SYSTEMS} local)
set(swcdb_fsLocal_sources
  ${SWC_VERSION_SRC}
  Local/FileSystem.cc
)

set(cross_target)
if(NOT SWC_BUILD_PKG AND NOT swcdb_fs_local IN_LIST BUILTIN_FS_TARGETS)
  set(cross_target swcdb_fs)
endif()

ADD_LIB_TARGET(
  NAME      swcdb_fs_local
  SRCS      ${swcdb_fsLocal_sources}
  TARGETS   ${SWC_LIB_CORE_TARGET}
            ${SWC_LIB_CORE_CONFIG_TARGET}
            ${cross_target}
  SHARED    ${SWC_LIB_CORE_LIBRARIES_SHARED}
            ${SWC_LIB_CORE_CONFIG_LIBRARIES_SHARED}
            ${SWC_LIB_FS_LIBRARIES_SHARED}
  STATIC    ${SWC_LIB_CORE_LIBRARIES_STATIC}
            ${SWC_LIB_CORE_CONFIG_LIBRARIES_STATIC}
            ${SWC_LIB_FS_LIBRARIES_STATIC}
)
#########################

endif()





if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "lib-fs-ceph")

########### CEPH FILESYSTEM ###########

if(CEPH_LIBRARIES_SHARED)
  set(FILE_SYSTEMS ${FILE_SYSTEMS} ceph)
  set(swcdb_fsCeph_sources
    ${SWC_VERSION_SRC}
    Ceph/FileSystem.cc
  )
  set(cross_target)
  if(NOT SWC_BUILD_PKG AND NOT swcdb_fs_ceph IN_LIST BUILTIN_FS_TARGETS)
    set(cross_target swcdb_fs)
  endif()
  if(NOT CEPH_LIBRARIES_STATIC)
    set(CEPH_LIBRARIES_STATIC ${CEPH_LIBRARIES_SHARED})
  endif()
  ADD_LIB_TARGET(
    NAME      swcdb_fs_ceph
    SRCS      ${swcdb_fsCeph_sources}
    TARGETS   ${SWC_LIB_CORE_TARGET}
              ${SWC_LIB_CORE_CONFIG_TARGET}
              ${cross_target}
    SHARED    ${SWC_LIB_CORE_LIBRARIES_SHARED}
              ${SWC_LIB_CORE_CONFIG_LIBRARIES_SHARED}
              ${SWC_LIB_FS_LIBRARIES_SHARED}
              ${CEPH_LIBRARIES_SHARED}
    STATIC    ${SWC_LIB_CORE_LIBRARIES_STATIC}
              ${SWC_LIB_CORE_CONFIG_LIBRARIES_STATIC}
              ${SWC_LIB_FS_LIBRARIES_STATIC}
              ${CEPH_LIBRARIES_STATIC}
  )
endif()
#########################

endif()





if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "lib-fs-hadoop")

########### HADOOP FILESYSTEM ###########

if(HADOOP_LIBRARIES_SHARED AND PROTOBUF_LIBRARIES_SHARED AND GSASL_LIBRARIES_SHARED)
  set(hadoop_name "hadoop")
  # if(HADOOP_VERSION)
  #  set(hadoop_name "${hadoop_name}_${HADOOP_VERSION}")
  # endif()

  set(FILE_SYSTEMS ${FILE_SYSTEMS} ${hadoop_name})
  set(swcdb_fsHadoop_sources
    ${SWC_VERSION_SRC}
    Hadoop/FileSystem.cc
  )

  set(cross_target)
  if(NOT SWC_BUILD_PKG AND NOT swcdb_fs_${hadoop_name} IN_LIST BUILTIN_FS_TARGETS)
    set(cross_target swcdb_fs)
  endif()

  ADD_LIB_TARGET(
    NAME      swcdb_fs_${hadoop_name}
    SRCS      ${swcdb_fsHadoop_sources}
    TARGETS   ${SWC_LIB_CORE_TARGET}
              ${SWC_LIB_CORE_CONFIG_TARGET}
              ${cross_target}
    SHARED    ${SWC_LIB_CORE_LIBRARIES_SHARED}
              ${SWC_LIB_CORE_CONFIG_LIBRARIES_SHARED}
              ${SWC_LIB_FS_LIBRARIES_SHARED}
              ${HADOOP_LIBRARIES_SHARED}
              ${PROTOBUF_LIBRARIES_SHARED}
              ${GSASL_LIBRARIES_SHARED}
    STATIC    ${SWC_LIB_CORE_LIBRARIES_STATIC}
              ${SWC_LIB_CORE_CONFIG_LIBRARIES_STATIC}
              ${SWC_LIB_FS_LIBRARIES_STATIC}
              ${HADOOP_LIBRARIES_STATIC}
              ${PROTOBUF_LIBRARIES_STATIC}
              ${GSASL_LIBRARIES_STATIC}
    SOVERSION ${HADOOP_VERSION}
  )

  if(BUILTIN_FS_HADOOP)
    set(SWC_LIB_FS_SHARED ${SWC_LIB_FS_SHARED}
                          ${HADOOP_LIBRARIES_SHARED}
                          ${PROTOBUF_LIBRARIES_SHARED}
                          ${GSASL_LIBRARIES_SHARED})
    set(SWC_LIB_FS_STATIC ${SWC_LIB_FS_STATIC}
                          ${HADOOP_LIBRARIES_STATIC}
                          ${PROTOBUF_LIBRARIES_STATIC}
                          ${GSASL_LIBRARIES_STATIC})
  endif()

endif()

#########################

endif()





if(NOT SWC_BUILD_PKG OR SWC_BUILD_PKG STREQUAL "lib-fs-hadoop-jvm")

########### HADOOP_JVM FILESYSTEM ###########

if(HADOOP_JVM_LIBRARIES_SHARED) # static require (AND JAVA_LIBRARIES_SHARED)
  set(FILE_SYSTEMS ${FILE_SYSTEMS} hadoop_jvm)
  set(swcdb_fsHadoop_jvm_sources
    ${SWC_VERSION_SRC}
    HadoopJVM/FileSystem.cc
  )

  set(cross_target)
  if(NOT SWC_BUILD_PKG AND NOT swcdb_fs_hadoop_jvm IN_LIST BUILTIN_FS_TARGETS)
    set(cross_target swcdb_fs)
  endif()

  ADD_LIB_TARGET(
    NAME      swcdb_fs_hadoop_jvm
    SRCS      ${swcdb_fsHadoop_jvm_sources}
    TARGETS   ${SWC_LIB_CORE_TARGET}
              ${SWC_LIB_CORE_CONFIG_TARGET}
              ${cross_target}
    SHARED    ${SWC_LIB_CORE_LIBRARIES_SHARED}
              ${SWC_LIB_CORE_CONFIG_LIBRARIES_SHARED}
              ${SWC_LIB_FS_LIBRARIES_SHARED}
              ${HADOOP_JVM_LIBRARIES_SHARED}
              ${JAVA_LIBRARIES_SHARED}
    STATIC    ${SWC_LIB_CORE_LIBRARIES_STATIC}
              ${SWC_LIB_CORE_CONFIG_LIBRARIES_STATIC}
              ${SWC_LIB_FS_LIBRARIES_STATIC}
              ${HADOOP_JVM_LIBRARIES_SHARED}
              ${JAVA_LIBRARIES_SHARED}
  )
endif()

#########################

endif()


set_property(GLOBAL PROPERTY SWC_FILE_SYSTEMS ${FILE_SYSTEMS})

if(SWC_LIB_FS_SHARED)
  set_property(GLOBAL PROPERTY SWC_LIB_FS_SHARED ${SWC_LIB_FS_SHARED})
endif()
if(SWC_LIB_FS_STATIC)
  set_property(GLOBAL PROPERTY SWC_LIB_FS_STATIC ${SWC_LIB_FS_STATIC})
endif()
