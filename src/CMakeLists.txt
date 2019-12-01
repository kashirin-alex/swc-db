#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#


set(TEST_DIFF ${CMAKE_SOURCE_DIR}/src/testutils/testdiff)


include_directories(
  cc
  cc/include
  ${CMAKE_CURRENT_BINARY_DIR}/cc
)

# add_subdirectory(cc/hypertable)

configure_file(
  cc/include/swcdb/core/Version.h.in 
  ${CMAKE_CURRENT_SOURCE_DIR}/cc/include/swcdb/core/Version.h @ONLY
)

add_subdirectory(cc/lib/swcdb)
add_subdirectory(cc/bin/swcdb)




