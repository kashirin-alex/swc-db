#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


#if(SWC_MALLOC_NOT_INSISTENT) 
#  set(swcdb_core_sources )
#else()
#  set(swcdb_core_sources Malloc.cc)
#endif()

set(swcdb_core_sources
  ${SWC_VERSION_SRC}
  ${swcdb_core_sources}
  Time.cc
  String.cc
  Error.cc
  Exception.cc
  Logger.cc
  Checksum.cc
  Encoder.cc
  BufferStream.cc
  Semaphore.cc
  Comparators.cc
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
  FLAGS     ${SWC_DEFAULT_PATH_FLAGS}
)


set(swcdb_core_comm_sources
  ${SWC_VERSION_SRC}
  comm/Settings.cc
  comm/SerializedServer.cc
  comm/SerializedClient.cc
  comm/ConfigSSL.cc
  comm/Resolver.cc
  comm/Event.cc
  comm/HeaderBufferInfo.cc
  comm/Header.cc
  comm/Buffers.cc
  comm/ConnHandler.cc
  comm/AppContext.cc
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

