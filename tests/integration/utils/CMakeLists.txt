#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
# License details at <https://github.com/kashirin-alex/swc-db/#license>



add_test(
  NAME    test-swcdb_cluster-start
  COMMAND ${CMAKE_INSTALL_PREFIX}/sbin/swcdb_cluster start --swc.cfg.path=${CMAKE_CURRENT_SOURCE_DIR}/
)



add_test(
  NAME    utils_shell_dbclient 
  COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/utils_shell_dbclient.sh ${CMAKE_INSTALL_PREFIX} ${CMAKE_CURRENT_SOURCE_DIR}
)

