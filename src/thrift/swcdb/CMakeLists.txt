#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#


# IF .thrift instruction change with version it is prefered .thrift by versions
set(VER_GEN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/thriftgen-${THRIFT_VERSION})



# SET THRIFT CPP BROKER & CLIENT SOURCES
if(THRIFT_CPP_FOUND)
  set(THRIFT_CPP_GEN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cc/include/swcdb/thrift)

  if(NOT EXISTS ${THRIFT_CPP_GEN_PATH}/gen-cpp)
    if(THRIFT_COMPILER_FOUND)
      execute_process(COMMAND rm -rf ${VER_GEN_PATH}/gen-cpp )
      execute_process(COMMAND mkdir -p ${VER_GEN_PATH}/gen-cpp )
      execute_process(COMMAND thrift -r --gen cpp -o ${VER_GEN_PATH}/
                      ${CMAKE_CURRENT_SOURCE_DIR}/Broker.thrift)
    endif()

    if(EXISTS ${VER_GEN_PATH}/gen-cpp)  # set current gen-cpp/ included path	
      execute_process(COMMAND rm -rf 	${THRIFT_CPP_GEN_PATH}/gen-cpp )
      execute_process(COMMAND cp -r 	${VER_GEN_PATH}/gen-cpp ${THRIFT_CPP_GEN_PATH}/)
    endif()
  endif()

  if(EXISTS ${THRIFT_CPP_GEN_PATH}/gen-cpp)
      set(THRIFT_CPP_GEN_PATH ${THRIFT_CPP_GEN_PATH} PARENT_SCOPE)
  endif()
endif(THRIFT_CPP_FOUND)



# SET THRIFT C CLIENT SOURCES
if(THRIFT_C_FOUND)
  set(THRIFT_C_GEN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../c/include/swcdb/thrift)

  if(NOT EXISTS ${THRIFT_C_GEN_PATH}/gen-c_glib)
    if(THRIFT_COMPILER_FOUND)
      execute_process(COMMAND rm -rf ${VER_GEN_PATH}/gen-c_glib )
      execute_process(COMMAND mkdir -p ${VER_GEN_PATH}/gen-c_glib )
      execute_process(COMMAND thrift -r --gen c_glib  -o ${VER_GEN_PATH}/
                      ${CMAKE_CURRENT_SOURCE_DIR}/Service.thrift)
    endif()

    if(EXISTS ${VER_GEN_PATH}/gen-c_glib) # set current gen-c_glib/ included path	
      execute_process(COMMAND rm -rf 	${THRIFT_C_GEN_PATH}/gen-c_glib )
      execute_process(COMMAND cp -r 	${VER_GEN_PATH}/gen-c_glib ${THRIFT_C_GEN_PATH}/)
    endif()
  endif()

  if(EXISTS ${THRIFT_C_GEN_PATH}/gen-c_glib)
      set(THRIFT_C_GEN_PATH ${THRIFT_C_GEN_PATH} PARENT_SCOPE)
  endif()
endif(THRIFT_C_FOUND)



# SET THRIFT PYTHON CLIENT MODULE
if(PYTHON_EXECUTABLES)
  set(THRIFT_PY_GEN_PATH ${CMAKE_CURRENT_BINARY_DIR}/../../py/swcdb/pkg/thrift)

  if(NOT EXISTS ${THRIFT_PY_GEN_PATH}/gen)
    if(THRIFT_COMPILER_FOUND)
      execute_process(COMMAND rm -rf ${VER_GEN_PATH}/gen-py )
      execute_process(COMMAND mkdir -p ${VER_GEN_PATH}/gen-py )
      execute_process(COMMAND thrift -r --gen py:slots,no_utf8strings -o ${VER_GEN_PATH}/
                      ${CMAKE_CURRENT_SOURCE_DIR}/Service.thrift)
    endif()

    if(EXISTS ${VER_GEN_PATH}/gen-py)  # set module path	
      execute_process(COMMAND mkdir -p ${THRIFT_PY_GEN_PATH} )
      execute_process(COMMAND rm -rf 	${THRIFT_PY_GEN_PATH}/gen-py )
      execute_process(COMMAND cp -r 	${VER_GEN_PATH}/gen-py/swcdb/thrift/gen ${THRIFT_PY_GEN_PATH}/)
    endif()
  endif()

  if(EXISTS ${THRIFT_PY_GEN_PATH}/gen)
      set(THRIFT_PY_GEN_PATH ${THRIFT_PY_GEN_PATH} PARENT_SCOPE)
  endif()
endif(PYTHON_EXECUTABLES)
