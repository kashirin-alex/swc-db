#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>



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
      foreach(col_type PLAIN;COUNTER;SERIAL)
        foreach(key_seq LEXIC;VOLUME;FC_LEXIC;FC_VOLUME)
          foreach(with_broker  false;true)

            add_test(
              NAME
                test-query-${col_type}-${key_seq}-v${versions}-c${ncells}-f${nfractions}-bkr_${with_broker}
              COMMAND
                query
                  --swc.cfg.path=${CMAKE_INSTALL_PREFIX}/etc/swcdb/
                  --col-type=${col_type}
                  --col-seq=${key_seq}
                  --cell-versions=${versions}
                  --ncells=${ncells}
                  --nfractions=${nfractions}
                  --with-broker=${with_broker}
            )

          endforeach()
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