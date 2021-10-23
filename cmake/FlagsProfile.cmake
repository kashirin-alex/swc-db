#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>



if(SWC_PROFILE AND
   (SWC_PROFILE STREQUAL "STACK" OR SWC_PROFILE STREQUAL "ALL"))
  if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-usage")
  endif()
endif()
