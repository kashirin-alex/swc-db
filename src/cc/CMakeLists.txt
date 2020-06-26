#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
# License details at <https://github.com/kashirin-alex/swc-db/#license>


SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}\
 -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/include/=\
 -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/lib/="
)


include_directories(
  include
  lib
)

add_subdirectory(lib/swcdb)
add_subdirectory(bin/swcdb)

install(DIRECTORY etc/swcdb      DESTINATION etc)
install(DIRECTORY include/swcdb  DESTINATION include)

