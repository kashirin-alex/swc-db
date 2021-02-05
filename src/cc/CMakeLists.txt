#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}\
 -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/include/=\
 -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/bin/=\
 -ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/lib/="
)

configure_file(
  include/swcdb/Version.h.in
  ${CMAKE_CURRENT_SOURCE_DIR}/include/swcdb/Version.h @ONLY
)
set_property(
  GLOBAL PROPERTY
  SWC_VERSION_SRC "${CMAKE_CURRENT_SOURCE_DIR}/include/swcdb/Version.cc"
)

include_directories(
  include
  lib
)

add_subdirectory(lib/swcdb)
add_subdirectory(bin/swcdb)
