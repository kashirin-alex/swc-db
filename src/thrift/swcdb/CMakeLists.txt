#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>


## IF .thrift instruction change with version it is prefered .thrift by versions
set(VER_GEN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/thriftgen-${THRIFT_VERSION})




######## SET THRIFT CPP BROKER & CLIENT SOURCES
if(THRIFT_CPP_FOUND)
  set(THRIFT_CPP_GEN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cc/include/swcdb/thrift)

  if(NOT EXISTS ${THRIFT_CPP_GEN_PATH}/gen-cpp OR NOT EXISTS ${VER_GEN_PATH}/gen-cpp)
    if(THRIFT_COMPILER_FOUND)
      execute_process(COMMAND rm -rf ${VER_GEN_PATH}/gen-cpp )
      execute_process(COMMAND mkdir -p ${VER_GEN_PATH}/gen-cpp )

      message(STATUS "Generating Thrift C++:")
      execute_process(COMMAND thrift -r --gen cpp:moveable_types
                      -o ${VER_GEN_PATH}/
                      ${CMAKE_CURRENT_SOURCE_DIR}/Broker.thrift)
      message("       ${VER_GEN_PATH}/gen-cpp")
    endif()

    if(EXISTS ${VER_GEN_PATH}/gen-cpp)  # set current gen-cpp/ included path
      execute_process(COMMAND rm -rf  ${THRIFT_CPP_GEN_PATH}/gen-cpp )
      execute_process(COMMAND cp -r  ${VER_GEN_PATH}/gen-cpp ${THRIFT_CPP_GEN_PATH}/)
    endif()

  endif()

  if(EXISTS ${THRIFT_CPP_GEN_PATH}/gen-cpp)
      set(THRIFT_CPP_GEN_PATH ${THRIFT_CPP_GEN_PATH} PARENT_SCOPE)
  endif()
endif(THRIFT_CPP_FOUND)



######## SET THRIFT C CLIENT SOURCES
if(THRIFT_C_FOUND)
  set(THRIFT_C_GEN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../c/include/swcdb/thrift)

  if(NOT EXISTS ${THRIFT_C_GEN_PATH}/gen-c_glib OR NOT EXISTS ${VER_GEN_PATH}/gen-c_glib)
    if(THRIFT_COMPILER_FOUND)
      execute_process(COMMAND rm -rf ${VER_GEN_PATH}/gen-c_glib )
      execute_process(COMMAND mkdir -p ${VER_GEN_PATH}/gen-c_glib )

      message(STATUS "Generating Thrift C-GLIB:")
      execute_process(COMMAND thrift -r --gen c_glib
                      -o ${VER_GEN_PATH}/
                      ${CMAKE_CURRENT_SOURCE_DIR}/Service.thrift)
      message("       ${VER_GEN_PATH}/gen-c_glib")
    endif()

    if(EXISTS ${VER_GEN_PATH}/gen-c_glib) # set current gen-c_glib/ included path
      execute_process(COMMAND rm -rf ${THRIFT_C_GEN_PATH}/gen-c_glib )
      execute_process(COMMAND cp -r  ${VER_GEN_PATH}/gen-c_glib ${THRIFT_C_GEN_PATH}/)
    endif()
  endif()

  if(EXISTS ${THRIFT_C_GEN_PATH}/gen-c_glib)
      set(THRIFT_C_GEN_PATH ${THRIFT_C_GEN_PATH} PARENT_SCOPE)
  endif()
endif(THRIFT_C_FOUND)



######## SET THRIFT JAVA CLIENT MODULE
if(LANGS OR LANG_JAVA)
  set(THRIFT_JAVA_GEN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../java/mvn/swcdb/thrift/src/main/java/org/swcdb/thrift/)

  if(NOT EXISTS ${THRIFT_JAVA_GEN_PATH}/gen OR NOT EXISTS ${VER_GEN_PATH}/gen-java)
    if(THRIFT_COMPILER_FOUND)
      execute_process(COMMAND rm -rf ${VER_GEN_PATH}/gen-java )
      execute_process(COMMAND mkdir -p ${VER_GEN_PATH}/gen-java )

      message(STATUS "Generating Thrift JAVA:")
      execute_process(COMMAND thrift -r --gen java:generated_annotations=suppress
                      -o ${VER_GEN_PATH}/
                      ${CMAKE_CURRENT_SOURCE_DIR}/Service.thrift)
      message("       ${VER_GEN_PATH}/gen-java")
    endif()

    if(EXISTS ${VER_GEN_PATH}/gen-java)  # set module path
      execute_process(COMMAND mkdir -p ${THRIFT_JAVA_GEN_PATH} )
      execute_process(COMMAND rm -rf ${THRIFT_JAVA_GEN_PATH}/gen )
      execute_process(COMMAND cp -r  ${VER_GEN_PATH}/gen-java/org/swcdb/thrift/gen ${THRIFT_JAVA_GEN_PATH}/)
    endif()
  endif()

  if(EXISTS ${THRIFT_JAVA_GEN_PATH}/gen)
      set(THRIFT_JAVA_GEN_PATH ${THRIFT_JAVA_GEN_PATH} PARENT_SCOPE)
  endif()
endif(LANGS OR LANG_JAVA)



######## SET THRIFT PYTHON CLIENT MODULE
if (LANGS OR LANG_PY2 OR LANG_PY3 OR LANG_PYPY2 OR LANG_PYPY3)
  set(THRIFT_PY_GEN_PATH ${CMAKE_CURRENT_BINARY_DIR}/../../py/package/swcdb/thrift)

  if(NOT EXISTS ${THRIFT_PY_GEN_PATH}/native OR NOT EXISTS ${VER_GEN_PATH}/gen-py)
    if(THRIFT_COMPILER_FOUND)

      set(GEN_TYPES "native;tornado;twisted;zope.interface")
      message(STATUS "Generating Thrift PYTHON: ${GEN_TYPES}")

      execute_process(COMMAND rm -rf ${VER_GEN_PATH}/gen-py )
      execute_process(COMMAND mkdir -p ${VER_GEN_PATH}/gen-py )

      foreach(typ ${GEN_TYPES})
        execute_process(COMMAND mkdir -p ${VER_GEN_PATH}/tmp )
        set(GEN_TYPE "")
        if(NOT typ STREQUAL "native")
          set(GEN_TYPE "${typ},")
        endif()
        execute_process(COMMAND thrift -r --gen py:${GEN_TYPE}slots,no_utf8strings
                        -o ${VER_GEN_PATH}/tmp/
                        ${CMAKE_CURRENT_SOURCE_DIR}/Service.thrift)

        set(GEN_TYPE "gen-py")
        if(NOT typ STREQUAL "native" AND NOT typ STREQUAL "zope.interface")
          set(GEN_TYPE ${GEN_TYPE}.${typ})
        endif()
        execute_process(COMMAND mv ${VER_GEN_PATH}/tmp/${GEN_TYPE} ${VER_GEN_PATH}/gen-py/${typ})
        execute_process(COMMAND rm -rf ${VER_GEN_PATH}/tmp)

        message("       ${VER_GEN_PATH}/gen-py/${typ}")
      endforeach()
    endif()

    if(EXISTS ${VER_GEN_PATH}/gen-py)  # set module path
      execute_process(COMMAND mkdir -p ${THRIFT_PY_GEN_PATH} )

      foreach(typ ${GEN_TYPES})
        if(typ STREQUAL "zope.interface")
          set(dest "zopeif")
        else()
          set(dest ${typ})
        endif()
        execute_process(COMMAND rm -rf ${THRIFT_PY_GEN_PATH}/${dest} )
        execute_process(COMMAND cp -r  ${VER_GEN_PATH}/gen-py/${typ}/swcdb/thrift/gen ${THRIFT_PY_GEN_PATH}/${dest}/)
      endforeach()
    endif()
  endif()

  if(EXISTS ${THRIFT_PY_GEN_PATH}/native)
      set(THRIFT_PY_GEN_PATH ${THRIFT_PY_GEN_PATH} PARENT_SCOPE)
  endif()
endif(LANGS OR LANG_PY2 OR LANG_PY3 OR LANG_PYPY2 OR LANG_PYPY3)



######## SET THRIFT NETSTD CLIENT SOURCES
if (LANGS OR LANG_NETSTD)
# set(THRIFT_NETSTD_GEN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../netstd/include/swcdb/thrift)

# if(NOT EXISTS ${THRIFT_NETSTD_GEN_PATH}/gen-netstd OR NOT EXISTS ${VER_GEN_PATH}/gen-netstd)
  if(NOT EXISTS ${VER_GEN_PATH}/gen-netstd)
    if(THRIFT_COMPILER_FOUND)
      execute_process(COMMAND rm -rf ${VER_GEN_PATH}/gen-netstd )
      execute_process(COMMAND mkdir -p ${VER_GEN_PATH}/gen-netstd )

      message(STATUS "Generating Thrift .NET Standard:")
      execute_process(COMMAND thrift -r --gen netstd
                      -o ${VER_GEN_PATH}/
                      ${CMAKE_CURRENT_SOURCE_DIR}/Service.thrift)
      message("       ${VER_GEN_PATH}/gen-netstd")
    endif()

#    if(EXISTS ${VER_GEN_PATH}/gen-netstd) # set current gen-netstd/ included path
#      execute_process(COMMAND rm -rf ${THRIFT_NETSTD_GEN_PATH}/gen-netstd )
#      execute_process(COMMAND mkdir -p ${THRIFT_NETSTD_GEN_PATH} )
#      execute_process(COMMAND cp -r  ${VER_GEN_PATH}/gen-netstd ${THRIFT_NETSTD_GEN_PATH}/)
#    endif()
  endif()

#  if(EXISTS ${THRIFT_NETSTD_GEN_PATH}/gen-netstd)
#      set(THRIFT_NETSTD_GEN_PATH ${THRIFT_NETSTD_GEN_PATH} PARENT_SCOPE)
# endif()
endif(LANGS OR LANG_NETSTD)



######## SET THRIFT RUST CLIENT SOURCES
if (LANGS OR LANG_RUST)
# set(THRIFT_RUST_GEN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../rs/include/swcdb/thrift)

# if(NOT EXISTS ${THRIFT_RUST_GEN_PATH}/gen-rs OR NOT EXISTS ${VER_GEN_PATH}/gen-rs)
  if(NOT EXISTS ${VER_GEN_PATH}/gen-rs)
    if(THRIFT_COMPILER_FOUND)
      execute_process(COMMAND rm -rf ${VER_GEN_PATH}/gen-rs )
      execute_process(COMMAND mkdir -p ${VER_GEN_PATH}/gen-rs )

      message(STATUS "Generating Thrift Rust:")
      execute_process(COMMAND thrift -r --gen rs
                      -o ${VER_GEN_PATH}/gen-rs
                      ${CMAKE_CURRENT_SOURCE_DIR}/Service.thrift)
      message("       ${VER_GEN_PATH}/gen-rs")
    endif()

#    if(EXISTS ${VER_GEN_PATH}/gen-rs) # set current gen-rs/ included path
#      execute_process(COMMAND rm -rf ${THRIFT_RUST_GEN_PATH}/gen-rs )
#      execute_process(COMMAND mkdir -p ${THRIFT_RUST_GEN_PATH} )
#      execute_process(COMMAND cp -r  ${VER_GEN_PATH}/gen-rs ${THRIFT_RUST_GEN_PATH}/)
#    endif()
  endif()

#  if(EXISTS ${THRIFT_RUST_GEN_PATH}/gen-rs)
#      set(THRIFT_RUST_GEN_PATH ${THRIFT_RUST_GEN_PATH} PARENT_SCOPE)
# endif()
endif(LANGS OR LANG_RUST)



######## SET THRIFT RUBY CLIENT SOURCES
if (LANGS OR LANG_RUBY)
  set(THRIFT_RUBY_GEN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../rb/package/lib/swcdb/thrift)

  if(NOT EXISTS ${THRIFT_RUBY_GEN_PATH}/gen OR NOT EXISTS ${VER_GEN_PATH}/gen-rb)
    if(THRIFT_COMPILER_FOUND)
      execute_process(COMMAND rm -rf ${VER_GEN_PATH}/gen-rb )
      execute_process(COMMAND mkdir -p ${VER_GEN_PATH}/gen-rb )

      message(STATUS "Generating Thrift Ruby:") # :rubygems,
      execute_process(COMMAND thrift -r --gen rb:namespaced
                      -o ${VER_GEN_PATH}
                      ${CMAKE_CURRENT_SOURCE_DIR}/Service.thrift)
      message("       ${VER_GEN_PATH}/gen-rb")
    endif()

    if(EXISTS ${VER_GEN_PATH}/gen-rb) # set current gen-rb/ included path
      execute_process(COMMAND rm -rf ${THRIFT_RUBY_GEN_PATH}/gen )
      execute_process(COMMAND mkdir -p ${THRIFT_RUBY_GEN_PATH} )
      execute_process(COMMAND cp -r  ${VER_GEN_PATH}/gen-rb/swcdb/thrift/gen ${THRIFT_RUBY_GEN_PATH}/)
    endif()
  endif()

  if(EXISTS ${THRIFT_RUBY_GEN_PATH}/gen)
      set(THRIFT_RUBY_GEN_PATH ${THRIFT_RUBY_GEN_PATH} PARENT_SCOPE)
  endif()
endif(LANGS OR LANG_RUBY)



######## SET THRIFT GO CLIENT SOURCES
if (LANGS OR LANG_GO)

  if(NOT EXISTS ${VER_GEN_PATH}/gen-go)
    if(THRIFT_COMPILER_FOUND)
      execute_process(COMMAND rm -rf ${VER_GEN_PATH}/gen-go )
      execute_process(COMMAND mkdir -p ${VER_GEN_PATH}/gen-go )

      message(STATUS "Generating Thrift GO:")
      execute_process(COMMAND thrift -r --gen go:case_sensitive
                      -o ${VER_GEN_PATH}
                      ${CMAKE_CURRENT_SOURCE_DIR}/Service.thrift)
      message("       ${VER_GEN_PATH}/gen-go")
    endif()

  endif()

endif(LANGS OR LANG_GO)
