#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${BUILTIN_FS_FLAGS}")



add_test(
  NAME    test-swcdb_cluster-deploy
  COMMAND ${CMAKE_INSTALL_PREFIX}/sbin/swcdb_cluster deploy
)

add_test(
  NAME    test-swcdb_cluster-start
  COMMAND ${CMAKE_INSTALL_PREFIX}/sbin/swcdb_cluster start
)

add_test(
  NAME    test-swcdb_cluster-stop
  COMMAND ${CMAKE_INSTALL_PREFIX}/sbin/swcdb_cluster stop
)



add_subdirectory("comm")

add_subdirectory("fs")

add_subdirectory("manager")

add_subdirectory("broker")

add_subdirectory("ranger")

add_subdirectory("client")

if(NOT SWC_WITHOUT_THRIFT)
  add_subdirectory("thrift")
endif()

add_subdirectory("utils")
