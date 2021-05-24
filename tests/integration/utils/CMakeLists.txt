#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>




add_test(
  NAME    utils_shell_dbclient
  COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/utils_shell_dbclient.sh ${CMAKE_INSTALL_PREFIX} ${CMAKE_CURRENT_SOURCE_DIR} 
)

add_test(
  NAME    utils_shell_dbclient_broker
  COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/utils_shell_dbclient.sh ${CMAKE_INSTALL_PREFIX} ${CMAKE_CURRENT_SOURCE_DIR} --with-broker
)

