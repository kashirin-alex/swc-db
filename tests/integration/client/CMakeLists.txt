#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#



add_test(
  NAME    test-swcdb_cluster-start
  COMMAND ${CMAKE_INSTALL_PREFIX}/sbin/swcdb_cluster start
)


ADD_TEST_EXEC(
  NAME      query
  SRCS      query.cc
  TARGETS   swcdb_core swcdb_core_config swcdb_core_comm swcdb
)

foreach(nfractions 1;26) #;159
  foreach(ncells 10;1000)
    foreach(versions 1;3;10)
      foreach(col_type PLAIN;COUNTER)
        foreach(key_seq LEXIC;VOLUME;FC_LEXIC;FC_VOLUME)

          add_test(
            NAME 
              test-query-${col_type}-${key_seq}-v${versions}-c${ncells}-f${nfractions}
            COMMAND 
              query
                --swc.cfg.path=${CMAKE_INSTALL_PREFIX}/etc/swcdb/
                --col-type=${col_type}  
                --col-seq=${key_seq} 
                --cell-versions=${versions} 
                --ncells=${ncells} 
                --nfractions=${nfractions} 
          )

        endforeach()
      endforeach()
    endforeach()
  endforeach()
endforeach()


#ADD_TEST_TARGET(
#  NAME      query
#  SRCS      test_query.cc
#  TARGETS   swcdb_core swcdb_core_config swcdb_core_comm swcdb
#  ARGS      "--swc.cfg.path=${CMAKE_INSTALL_PREFIX}/etc/swcdb/"
#)

add_test(
  NAME    test-swcdb_cluster-stop
  COMMAND ${CMAKE_INSTALL_PREFIX}/sbin/swcdb_cluster stop
)