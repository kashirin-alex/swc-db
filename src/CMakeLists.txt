#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
# License details at <https://github.com/kashirin-alex/swc-db/#license>



add_subdirectory(thrift/swcdb)


add_subdirectory(cc)

add_subdirectory(c)

add_subdirectory(py)



install(DIRECTORY cc/etc/swcdb      DESTINATION etc)


install(FILES     cc/include/swcdb/Version.h  DESTINATION include/swcdb)
install(DIRECTORY cc/include/swcdb/core       DESTINATION include/swcdb)
install(DIRECTORY cc/include/swcdb/common     DESTINATION include/swcdb)
install(DIRECTORY cc/include/swcdb/fs         DESTINATION include/swcdb)
install(DIRECTORY cc/include/swcdb/db         DESTINATION include/swcdb)
install(DIRECTORY cc/include/swcdb/utils      DESTINATION include/swcdb)


if(THRIFT_CPP_GEN_PATH)
  install(DIRECTORY cc/include/swcdb/thrift/client      DESTINATION include/swcdb/thrift)
  install(DIRECTORY cc/include/swcdb/thrift/utils       DESTINATION include/swcdb/thrift)
  install(DIRECTORY cc/include/swcdb/thrift/gen-cpp     DESTINATION include/swcdb/thrift)
endif(THRIFT_CPP_GEN_PATH)


if(THRIFT_C_GEN_PATH)
  install(FILES     c/include/swcdb/version.h           DESTINATION include/swcdb)
  install(DIRECTORY c/include/swcdb/thrift/client       DESTINATION include/swcdb/thrift)
  install(DIRECTORY c/include/swcdb/thrift/gen-c_glib   DESTINATION include/swcdb/thrift)
endif(THRIFT_C_GEN_PATH)

