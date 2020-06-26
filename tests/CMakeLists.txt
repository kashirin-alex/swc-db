#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
# License details at <https://github.com/kashirin-alex/swc-db/#license>


SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}\
 -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/include/=\
 -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/lib/="
)


include_directories(
  ../src/cc/include
  ../src/cc/lib
)

set(TEST_DIFF ${CMAKE_CURRENT_SOURCE_DIR}/testutils/testdiff)

get_property(SWC_LIB_CORE_SHARED GLOBAL PROPERTY SWC_LIB_CORE_SHARED)
get_property(SWC_LIB_CORE_STATIC GLOBAL PROPERTY SWC_LIB_CORE_STATIC)


add_subdirectory("libswcdb_core")


add_subdirectory("libswcdb")


add_subdirectory("integration")



