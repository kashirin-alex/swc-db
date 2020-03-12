#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#


set(TEST_DIFF ${CMAKE_SOURCE_DIR}/src/testutils/testdiff)


include_directories(
  cc/include
  cc/lib
  ${CMAKE_CURRENT_BINARY_DIR}/cc
)


configure_file(
  cc/include/swcdb/core/Version.h.in 
  ${CMAKE_CURRENT_SOURCE_DIR}/cc/include/swcdb/core/Version.h @ONLY
)

add_subdirectory(cc/lib/swcdb)
add_subdirectory(cc/bin/swcdb)


install(DIRECTORY cc/etc/swcdb      DESTINATION etc)
install(DIRECTORY cc/include/swcdb  DESTINATION include)


add_subdirectory(py)