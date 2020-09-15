#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>



ADD_TEST_EXEC(
  NAME      fs_client
  SRCS      test_fs_client.cc
  TARGETS   swcdb_core swcdb_core_config swcdb_core_comm swcdb_fs ${BUILTIN_FS_TARGETS}   
  ONLY_DYN  TRUE
)

get_property(FILE_SYSTEMS GLOBAL PROPERTY SWC_FILE_SYSTEMS)

foreach(fs ${FILE_SYSTEMS})
  if(fs STREQUAL "broker")
    add_test(
      NAME    test-swcdb_cluster-start_fsbrokers
      COMMAND ${CMAKE_INSTALL_PREFIX}/sbin/swcdb_cluster start-fsbrokers
    )
  endif()
  
  add_test(NAME test-fs_client-${fs}
    COMMAND fs_client 
      --swc.fs=${fs} 
      --swc.fs.lib.${fs}=${CMAKE_INSTALL_PREFIX}/lib/libswcdb_fs_${fs}.so
      --swc.cfg.path=${CMAKE_INSTALL_PREFIX}/etc/swcdb/
  )

  if(fs STREQUAL "broker")
    add_test(
      NAME    test-swcdb_cluster-stop_fsbrokers
      COMMAND ${CMAKE_INSTALL_PREFIX}/sbin/swcdb_cluster stop-fsbrokers
    )
  endif()
endforeach()
