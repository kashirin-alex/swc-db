#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#

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



