#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}\
 -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/include/=\
 -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/lib/="
)


include_directories(
  ../src/cc/include
  ../src/cc/lib
)

set(TEST_DIFF bash ${CMAKE_CURRENT_SOURCE_DIR}/testutils/testdiff)

get_property(SWC_VERSION_SRC GLOBAL PROPERTY SWC_VERSION_SRC)
get_property(SWC_LIB_CORE_SHARED GLOBAL PROPERTY SWC_LIB_CORE_SHARED)
get_property(SWC_LIB_CORE_STATIC GLOBAL PROPERTY SWC_LIB_CORE_STATIC)

if(SWC_ENABLE_SANITIZER)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSWC_ENABLE_SANITIZER")
endif()


add_subdirectory("libswcdb_core")


add_subdirectory("libswcdb")


add_subdirectory("integration")



