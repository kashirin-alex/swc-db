#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#

if(THRIFT_FOUND)

set(thrift_ver_gen_path ${CMAKE_CURRENT_SOURCE_DIR}/thriftgen-${THRIFT_VERSION})
set(thrift_incl_path ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/swcdb/thrift)

# IF .thirft instruction change with version it is prefered .thrift by versions

execute_process(COMMAND rm -rf ${thrift_ver_gen_path}/gen-cpp ) ## temporary

if(NOT EXISTS ${thrift_ver_gen_path}/gen-cpp)
	execute_process(COMMAND mkdir -p ${thrift_ver_gen_path}/gen-cpp ) ## temporary
  execute_process(COMMAND thrift -r --gen cpp 
                  -o ${thrift_ver_gen_path}/
                  ${CMAKE_CURRENT_SOURCE_DIR}/Broker.thrift)
endif ()

# set current gen-cpp/ included path	
execute_process(COMMAND rm -rf 	${thrift_incl_path}/gen-cpp )
execute_process(COMMAND cp -r 	${thrift_ver_gen_path}/gen-cpp ${thrift_incl_path}/)



# libswcdb_thrift
ADD_LIB_TARGET(
	NAME 	swcdb_thrift
	SRCS 	${thrift_incl_path}/gen-cpp/Service_constants.cpp 
        ${thrift_incl_path}/gen-cpp/Service_types.cpp
        ${thrift_incl_path}/gen-cpp/Service.cpp 
        # Converters.cc 
        # Client.cc
	SHARED ${THRIFT_LIBRARIES_SHARED}
	STATIC ${THRIFT_LIBRARIES_STATIC}
)

ADD_TEST_TARGET(
	NAME thrift_client
	SRCS client/tests/client.cc
	TARGETS swcdb_thrift 
)


endif()
