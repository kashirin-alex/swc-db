#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


get_property(SWC_VERSION_SRC GLOBAL PROPERTY SWC_VERSION_SRC)

get_property(SWC_LIB_CORE_SHARED GLOBAL PROPERTY SWC_LIB_CORE_SHARED)
get_property(SWC_LIB_CORE_STATIC GLOBAL PROPERTY SWC_LIB_CORE_STATIC)

get_property(SWC_LIB_FS_SHARED GLOBAL PROPERTY SWC_LIB_FS_SHARED)
get_property(SWC_LIB_FS_STATIC GLOBAL PROPERTY SWC_LIB_FS_STATIC)



set(SHARED ${SWC_LIB_CORE_SHARED} ${SWC_LIB_FS_SHARED})
set(STATIC ${SWC_LIB_CORE_STATIC} ${SWC_LIB_FS_STATIC})

set(FLAGS )
set(TARGETS )

set(SHARED_TARGETS
  "swcdb_core;swcdb_core_config;swcdb_core_comm;swcdb_fs;swcdb;${BUILTIN_FS_TARGETS}")

if(SWC_IMPL_SOURCE)
  set(FLAGS "-DSWC_IMPL_SOURCE")
else()
  set(TARGETS ${SHARED_TARGETS})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${BUILTIN_FS_FLAGS}")



#add_subdirectory(manager)
#add_subdirectory(ranger)
#add_subdirectory(thriftbroker)



# swcdbRanger
ADD_EXEC_TARGET(
  NAME    swcdbRanger
  SRCS    ranger/main.cc
          ${SWC_VERSION_SRC}
  TARGETS ${TARGETS}

  SHARED  ${SHARED}
  STATIC  ${STATIC}
  FLAGS   ${FLAGS}
)


# swcdbManager
ADD_EXEC_TARGET(
  NAME    swcdbManager
  SRCS    manager/main.cc
          ${SWC_VERSION_SRC}
  TARGETS ${TARGETS}

  SHARED  ${SHARED}
  STATIC  ${STATIC}
  FLAGS   ${FLAGS}
)


# swcdbThriftBroker
if(THRIFT_CPP_GEN_PATH)
ADD_EXEC_TARGET(
  NAME    swcdbThriftBroker
  SRCS    thriftbroker/main.cc
          ${SWC_VERSION_SRC}
  TARGETS ${TARGETS}

  SHARED ${SHARED} ${THRIFT_CPP_LIBRARIES_SHARED}
  STATIC ${STATIC} ${THRIFT_CPP_LIBRARIES_STATIC}
  FLAGS  ${FLAGS} -Wno-suggest-override -Wno-old-style-cast -Wno-noexcept
)
endif()


# swcdbFsBroker
ADD_EXEC_TARGET(
  NAME    swcdbFsBroker
  SRCS    fsbroker/main.cc
          ${SWC_VERSION_SRC}
  TARGETS swcdb_core
          swcdb_core_config
          swcdb_core_comm
          swcdb_fs
          ${BUILTIN_FS_TARGETS}

  SHARED  ${SHARED}
  STATIC  ${STATIC}
  FLAGS   "-DFS_BROKER_APP"

  ONLY_DYN TRUE
)


# swcdb
ADD_EXEC_TARGET(
  NAME    swcdb
  SRCS    utils/main.cc
          ${SWC_VERSION_SRC}
  TARGETS ${SHARED_TARGETS}

  SHARED  ${SHARED}
  STATIC  ${STATIC}

  ONLY_DYN TRUE
)


# swcdb_load_generator
ADD_EXEC_TARGET(
  NAME    swcdb_load_generator
  SRCS    utils/load_generator.cc
          ${SWC_VERSION_SRC}
  TARGETS ${TARGETS}

  SHARED  ${SHARED}
  STATIC  ${STATIC}
  FLAGS   ${FLAGS}
)
