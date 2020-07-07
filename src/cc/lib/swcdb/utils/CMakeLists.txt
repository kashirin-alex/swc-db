#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
# License details at <https://github.com/kashirin-alex/swc-db/#license>




########### SHELL UTILITY ###########

if(USE_GNU_READLINE)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_GNU_READLINE")
endif()

set(swcdb_utils_shell_sources
  ${SWC_VERSION_SRC}
  cli/Shell.cc
)
ADD_LIB_TARGET(
  NAME    swcdb_utils_shell
  SRCS    ${swcdb_utils_shell_sources}
  TARGETS 
  
  SHARED  ${EDITLINE_LIBRARIES_SHARED}
  STATIC  ${EDITLINE_LIBRARIES_STATIC}
)
