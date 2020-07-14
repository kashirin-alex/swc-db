#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
# License details at <https://github.com/kashirin-alex/swc-db/#license>


## IF .thrift instruction change with version it is prefered .thrift by versions
set(VER_GEN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/thriftgen-${THRIFT_VERSION})

set(found_version_gen FALSE)
if(EXISTS ${VER_GEN_PATH})
  set(found_version_gen TRUE)
endif()


######## SET THRIFT CPP BROKER & CLIENT SOURCES
if(THRIFT_CPP_FOUND)
  set(THRIFT_CPP_GEN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cc/include/swcdb/thrift)

  if(NOT EXISTS ${THRIFT_CPP_GEN_PATH}/gen-cpp OR NOT ${found_version_gen})
    if(THRIFT_COMPILER_FOUND)
      execute_process(COMMAND rm -rf ${VER_GEN_PATH}/gen-cpp )
      execute_process(COMMAND mkdir -p ${VER_GEN_PATH}/gen-cpp )

      message(STATUS "Generating Thrift C++:")
      execute_process(COMMAND thrift -r --gen cpp -o ${VER_GEN_PATH}/
                      ${CMAKE_CURRENT_SOURCE_DIR}/Broker.thrift)
      message(STATUS "    ${VER_GEN_PATH}/gen-cpp")
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

  if(NOT EXISTS ${THRIFT_C_GEN_PATH}/gen-c_glib OR NOT ${found_version_gen})
    if(THRIFT_COMPILER_FOUND)
      execute_process(COMMAND rm -rf ${VER_GEN_PATH}/gen-c_glib )
      execute_process(COMMAND mkdir -p ${VER_GEN_PATH}/gen-c_glib )

      message(STATUS "Generating Thrift C-GLIB:")
      execute_process(COMMAND thrift -r --gen c_glib  -o ${VER_GEN_PATH}/
                      ${CMAKE_CURRENT_SOURCE_DIR}/Service.thrift)
      message(STATUS "    ${VER_GEN_PATH}/gen-c_glib")
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
if(MAVEN_FOUND)
  set(THRIFT_JAVA_GEN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../java/mvn/swcdb/thrift/src/main/java/org/swcdb/thrift/)
  
  if(NOT EXISTS ${THRIFT_JAVA_GEN_PATH}/gen OR NOT ${found_version_gen})
    if(THRIFT_COMPILER_FOUND)
      execute_process(COMMAND rm -rf ${VER_GEN_PATH}/gen-java )
      execute_process(COMMAND mkdir -p ${VER_GEN_PATH}/gen-java )
      
      message(STATUS "Generating Thrift JAVA:")
      execute_process(COMMAND thrift -r --gen java -o ${VER_GEN_PATH}/
                      ${CMAKE_CURRENT_SOURCE_DIR}/Service.thrift)
      message(STATUS "    ${VER_GEN_PATH}/gen-java")
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
endif(MAVEN_FOUND)



######## SET THRIFT PYTHON CLIENT MODULE
if(PYTHON_EXECUTABLES)
  set(THRIFT_PY_GEN_PATH ${CMAKE_CURRENT_BINARY_DIR}/../../py/swcdb/pkg/thrift)

  if(NOT EXISTS ${THRIFT_PY_GEN_PATH}/gen OR NOT ${found_version_gen})
    if(THRIFT_COMPILER_FOUND)
      execute_process(COMMAND rm -rf ${VER_GEN_PATH}/gen-py )
      execute_process(COMMAND mkdir -p ${VER_GEN_PATH}/gen-py )

      message(STATUS "Generating Thrift PYTHON:")
      execute_process(COMMAND thrift -r --gen py:slots,no_utf8strings -o ${VER_GEN_PATH}/
                      ${CMAKE_CURRENT_SOURCE_DIR}/Service.thrift)
      message(STATUS "    ${VER_GEN_PATH}/gen-py")
    endif()

    if(EXISTS ${VER_GEN_PATH}/gen-py)  # set module path
      execute_process(COMMAND mkdir -p ${THRIFT_PY_GEN_PATH} )
      execute_process(COMMAND rm -rf ${THRIFT_PY_GEN_PATH}/gen-py )
      execute_process(COMMAND cp -r  ${VER_GEN_PATH}/gen-py/swcdb/thrift/gen ${THRIFT_PY_GEN_PATH}/)
    endif()
  endif()

  if(EXISTS ${THRIFT_PY_GEN_PATH}/gen)
      set(THRIFT_PY_GEN_PATH ${THRIFT_PY_GEN_PATH} PARENT_SCOPE)
  endif()
endif(PYTHON_EXECUTABLES)
