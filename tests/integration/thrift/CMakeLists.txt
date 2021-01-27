#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


add_test(
  NAME    test-swcdb_cluster-start
  COMMAND ${CMAKE_INSTALL_PREFIX}/sbin/swcdb_cluster start
)


###
if(THRIFT_CPP_FOUND)
  ADD_TEST_TARGET(
    NAME      thrift_cc_client
    SRCS      client.cc
    TARGETS   swcdb_thrift
    FLAGS     -Wno-error=suggest-override -Wno-error=noexcept
  )
endif()


###
if(THRIFT_C_FOUND)
  include_directories(
    ../../../src/c/include
    ../../../src/c/lib
    ${GLIB_INCLUDE_PATHS}/glib-2.0
    ${GLIB_INCLUDE_PATHS}/../lib/glib-2.0/include/
  )

  # gcc-10.2, pcre/sljit/sljitNativeX86_64.c:183:12: error: writing 1 byte into a region of size 0
  SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-stringop-overflow")
  # set(TEST_LINKING SHARED)
  ADD_TEST_TARGET(
    NAME      thrift_c_client_c
    SRCS      client.c
    TARGETS   swcdb_thrift_c
  )
endif()


###
get_property(THRIFT_JAVA_TEST GLOBAL PROPERTY THRIFT_JAVA_TEST)
if(THRIFT_JAVA_TEST)
  add_test(thrift_java_client
	  java -cp
      ${CMAKE_INSTALL_PREFIX}/lib/java/swcdb-thrift-${SWCDB_VERSION}-v${THRIFT_VERSION}-bundled.jar:${THRIFT_JAVA_TEST}
      org.swcdb.thrift.Tests
  )
endif()


add_test(
  NAME    test-swcdb_cluster-stop
  COMMAND ${CMAKE_INSTALL_PREFIX}/sbin/swcdb_cluster stop
)