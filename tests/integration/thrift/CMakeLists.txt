#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
# License details at <https://github.com/kashirin-alex/swc-db/#license>


add_test(
  NAME    test-swcdb_cluster-start
  COMMAND ${CMAKE_INSTALL_PREFIX}/sbin/swcdb_cluster start
)



if(THRIFT_CPP_FOUND)
  ADD_TEST_TARGET(
    NAME      thrift_cc_client
    SRCS      client.cc
    TARGETS   swcdb_thrift 
  )
endif()


if(THRIFT_C_FOUND)
  include_directories(
    ../../../src/c/include
    ../../../src/c/lib
    ${GLIB_INCLUDE_PATHS}/glib-2.0
    ${GLIB_INCLUDE_PATHS}/../lib/glib-2.0/include/
  )
  ADD_TEST_TARGET(
    NAME      thrift_c_client_c
    SRCS      client.c
    TARGETS   swcdb_thrift_c
  )
endif()



add_test(
  NAME    test-swcdb_cluster-stop
  COMMAND ${CMAKE_INSTALL_PREFIX}/sbin/swcdb_cluster stop
)