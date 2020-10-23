#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


set(FILE_SYSTEMS "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${BUILTIN_FS_FLAGS}")




########### FILESYSTEM CORE/BASE ###########

set(swcdb_fscore_sources
  ${SWC_VERSION_SRC}
  Dirent.cc
  SmartFd.cc
  Settings.cc
  FileSystem.cc
  Interface.cc
)
ADD_LIB_TARGET(
  NAME      swcdb_fs
  SRCS      ${swcdb_fscore_sources}
  TARGETS   swcdb_core swcdb_core_config
)
#########################



########### FSBROKER FILESYSTEM ###########

set(FILE_SYSTEMS ${FILE_SYSTEMS} broker)
set(swcdb_fsBroker_sources
  ${SWC_VERSION_SRC}
  Broker/FileSystem.cc
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

  Broker/Protocol/req/Base.cc
)
ADD_LIB_TARGET(
  NAME      swcdb_fs_broker
  SRCS      ${swcdb_fsBroker_sources}
  TARGETS   swcdb_fs swcdb_core swcdb_core_config swcdb_core_comm
)
#########################



########### LOCAL FILESYSTEM ###########    

set(FILE_SYSTEMS ${FILE_SYSTEMS} local)
set(swcdb_fsLocal_sources
  ${SWC_VERSION_SRC}
  Local/FileSystem.cc
)
ADD_LIB_TARGET(
  NAME      swcdb_fs_local
  SRCS      ${swcdb_fsLocal_sources}
  TARGETS   swcdb_fs swcdb_core swcdb_core_config
)
#########################



########### CEPH FILESYSTEM ###########

if(CEPH_LIBRARIES_SHARED)
  set(FILE_SYSTEMS ${FILE_SYSTEMS} ceph)
  set(swcdb_fsCeph_sources
    ${SWC_VERSION_SRC}
    Ceph/FileSystem.cc
  )
  if(NOT CEPH_LIBRARIES_STATIC)
    set(CEPH_LIBRARIES_STATIC ${CEPH_LIBRARIES_SHARED})
  endif()
  ADD_LIB_TARGET(
    NAME     swcdb_fs_ceph
    SRCS     ${swcdb_fsCeph_sources}
    TARGETS  swcdb_fs swcdb_core swcdb_core_config
    SHARED    ${CEPH_LIBRARIES_SHARED}
    STATIC    ${CEPH_LIBRARIES_STATIC}
  )
endif()
#########################



########### HADOOP FILESYSTEM ###########

if(HADOOP_LIBRARIES_SHARED AND PROTOBUF_LIBRARIES_SHARED AND GSASL_LIBRARIES_SHARED)
  set(haddop_name "hadoop")
  if(HADOOP_VERSION)
    set(haddop_name "${haddop_name}_${HADOOP_VERSION}")
  endif()

  set(FILE_SYSTEMS ${FILE_SYSTEMS} ${haddop_name})
  set(swcdb_fsHadoop_sources
    ${SWC_VERSION_SRC}
    Hadoop/FileSystem.cc
  )

  ADD_LIB_TARGET(
    NAME      swcdb_fs_${haddop_name}
    SRCS      ${swcdb_fsHadoop_sources}
    TARGETS   swcdb_fs
    SHARED    ${HADOOP_LIBRARIES_SHARED} 
              ${PROTOBUF_LIBRARIES_SHARED} 
              ${GSASL_LIBRARIES_SHARED}
    STATIC    ${HADOOP_LIBRARIES_STATIC} 
              ${PROTOBUF_LIBRARIES_STATIC}
              ${GSASL_LIBRARIES_STATIC}
  )


  set(SWC_LIB_FS_SHARED )
  set(SWC_LIB_FS_STATIC )
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

  if(SWC_LIB_FS_SHARED)
    set_property(GLOBAL PROPERTY SWC_LIB_FS_SHARED ${SWC_LIB_FS_SHARED})
  endif()
  if(SWC_LIB_FS_STATIC)
    set_property(GLOBAL PROPERTY SWC_LIB_FS_STATIC ${SWC_LIB_FS_STATIC})
  endif()
endif()

#########################



########### HADOOP_JVM FILESYSTEM ###########

if(HADOOP_JVM_LIBRARIES_SHARED) # static require (AND JAVA_LIBRARIES_SHARED)
  set(FILE_SYSTEMS ${FILE_SYSTEMS} hadoop_jvm)
  set(swcdb_fsHadoop_jvm_sources
    ${SWC_VERSION_SRC}
    HadoopJVM/FileSystem.cc
  )
  ADD_LIB_TARGET(
    NAME      swcdb_fs_hadoop_jvm
    SRCS      ${swcdb_fsHadoop_jvm_sources}
    TARGETS   swcdb_fs
    SHARED    ${HADOOP_JVM_LIBRARIES_SHARED} ${JAVA_LIBRARIES_SHARED} 
    STATIC    ${HADOOP_JVM_LIBRARIES_SHARED} ${JAVA_LIBRARIES_SHARED} 
  )
endif()

#########################


set_property(GLOBAL PROPERTY SWC_FILE_SYSTEMS ${FILE_SYSTEMS})
