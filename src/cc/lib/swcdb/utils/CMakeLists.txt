#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>




########### SHELL UTILITY ###########

if(USE_GNU_READLINE)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_GNU_READLINE")
endif()
if(USE_REPLXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_REPLXX")
endif()

if(SWC_ENABLE_SANITIZER)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSWC_ENABLE_SANITIZER")
endif()

set(swcdb_utils_shell_sources
  ${SWC_VERSION_SRC}
  cli/Shell.cc
  cli/Shell_DbClient.cc
  cli/Shell_Manager.cc
  cli/Shell_Ranger.cc
  cli/Shell_Fs.cc
  cli/Shell_Statistics.cc
)

ADD_LIB_TARGET(
  NAME      swcdb_utils_shell
  SRCS      ${swcdb_utils_shell_sources}
  TARGETS   ${SWC_LIB_CORE_TARGET}
            ${SWC_LIB_CORE_CONFIG_TARGET}
            ${SWC_LIB_CORE_COMM_TARGET}
            ${SWC_LIB_TARGET}
            ${SWC_LIB_FS_TARGET}
  SHARED    ${SWC_LIB_CORE_LIBRARIES_SHARED}
            ${SWC_LIB_CORE_CONFIG_LIBRARIES_SHARED}
            ${SWC_LIB_CORE_COMM_LIBRARIES_SHARED}
            ${SWC_LIB_LIBRARIES_SHARED}
            ${SWC_LIB_FS_LIBRARIES_SHARED}
            ${EDITLINE_LIBRARIES_SHARED}
  STATIC    ${SWC_LIB_CORE_LIBRARIES_STATIC}
            ${SWC_LIB_CORE_CONFIG_LIBRARIES_STATIC}
            ${SWC_LIB_CORE_COMM_LIBRARIES_STATIC}
            ${SWC_LIB_LIBRARIES_STATIC}
            ${SWC_LIB_FS_LIBRARIES_STATIC}
            ${EDITLINE_LIBRARIES_STATIC}
)
