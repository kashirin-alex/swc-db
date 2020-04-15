#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#

SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS}\
 -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/include/=\
 -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/lib/="
)

include_directories(
  include
  lib
  ${GLIB_INCLUDE_PATHS}/glib-2.0
  ${GLIB_INCLUDE_PATHS}/../lib/glib-2.0/include/
  
)

add_subdirectory(lib/swcdb)

install(DIRECTORY include/swcdb  DESTINATION include)


