#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
# License details at <https://github.com/kashirin-alex/swc-db/#license>


set(SWC_LIB_CORE_SHARED ${RE2_LIBRARIES_SHARED} 
                        ${ZLIB_LIBRARIES_SHARED} ${SNAPPY_LIBRARIES_SHARED} ${ZSTD_LIBRARIES_SHARED})
set(SWC_LIB_CORE_STATIC ${RE2_LIBRARIES_STATIC} 
                        ${ZLIB_LIBRARIES_STATIC} ${SNAPPY_LIBRARIES_STATIC} ${ZSTD_LIBRARIES_STATIC})

# ${ATOMIC_LIBRARIES_SHARED} ${BZIP2_LIBRARIES_SHARED} ${LZMA_LIBRARIES_SHARED} 
# ${ATOMIC_LIBRARIES_STATIC} ${BZIP2_LIBRARIES_STATIC} ${LZMA_LIBRARIES_STATIC}

set_property(GLOBAL PROPERTY SWC_LIB_CORE_SHARED ${SWC_LIB_CORE_SHARED} ${SSL_LIBRARIES_SHARED} )
set_property(GLOBAL PROPERTY SWC_LIB_CORE_STATIC ${SWC_LIB_CORE_STATIC} ${SSL_LIBRARIES_STATIC} )



set(swcdb_core_sources
  ${SWC_VERSION_SRC}
  Time.cc
  String.cc
  Error.cc
  Logger.cc
  Checksum.cc
  Encoder.cc
  FileUtils.cc
  Semaphore.cc
)
ADD_LIB_TARGET(
  NAME      swcdb_core
  SRCS      ${swcdb_core_sources}
  SHARED    ${SWC_LIB_CORE_SHARED}
  STATIC    ${SWC_LIB_CORE_STATIC}
)


set(swcdb_core_config_sources
  ${SWC_VERSION_SRC}
  config/Settings.cc
  config/Properties.cc
  config/Property.cc
  config/PropertiesParser.cc
)
ADD_LIB_TARGET(
  NAME      swcdb_core_config
  SRCS      ${swcdb_core_config_sources}
  TARGETS   swcdb_core
)


set(swcdb_core_comm_sources
  ${SWC_VERSION_SRC}
  comm/Settings.cc
  comm/SerializedServer.cc
  comm/SerializedClient.cc
  comm/ConfigSSL.cc
  comm/Resolver.cc
  comm/Event.cc
  comm/CommHeader.cc
  comm/CommBuf.cc
  comm/ConnHandler.cc
  comm/AppContext.cc
  comm/DispatchHandler.cc
  comm/ResponseCallback.cc
  comm/IoContext.cc
  comm/PeriodicTimer.cc
  comm/ClientConnQueues.cc
  comm/ClientConnQueue.cc
)
ADD_LIB_TARGET(
  NAME      swcdb_core_comm
  SRCS      ${swcdb_core_comm_sources}
  TARGETS   swcdb_core swcdb_core_config
  SHARED    ${SSL_LIBRARIES_SHARED} 
  STATIC    ${SSL_LIBRARIES_STATIC}
)

