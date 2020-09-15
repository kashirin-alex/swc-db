#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>



SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS}\
 -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/include/=\
 -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/lib/=\
 -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/bin/="
)

configure_file(
  include/swcdb/version.h.in 
  ${CMAKE_CURRENT_SOURCE_DIR}/include/swcdb/version.h @ONLY
)
set(SWC_VERSION_SRC "${CMAKE_CURRENT_SOURCE_DIR}/include/swcdb/version.c")



include_directories(
  include
  lib
  ${GLIB_INCLUDE_PATHS}/glib-2.0
  ${GLIB_INCLUDE_PATHS}/../lib/glib-2.0/include/
  
)

add_subdirectory(lib/swcdb)



