#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>



foreach(col_type PLAIN;COUNTER;SERIAL)
  foreach(by ID;NAME)
    foreach(ext plain;zst)

      add_test(
        NAME    utils_shell_dbclient-${col_type}-${by}-${ext}-bkr_false
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/utils_shell_dbclient.sh ${CMAKE_INSTALL_PREFIX} ${CMAKE_CURRENT_SOURCE_DIR} ${col_type} ${by} ${ext}
      )

      add_test(
        NAME    utils_shell_dbclient-${col_type}-${by}-${ext}-bkr_true
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/utils_shell_dbclient.sh ${CMAKE_INSTALL_PREFIX} ${CMAKE_CURRENT_SOURCE_DIR} ${col_type} ${by} ${ext} --with-broker
      )

    endforeach()
  endforeach()
endforeach()
