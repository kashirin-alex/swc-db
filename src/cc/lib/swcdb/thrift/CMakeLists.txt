#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#

if(THRIFT_FOUND)

# IF .thirft instruction change with version it is prefered .thrift by versions
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/thriftgen-${THRIFT_VERSION})
	execute_process(COMMAND  mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/thriftgen-${THRIFT_VERSION})
endif()

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/thriftgen-${THRIFT_VERSION}/gen-cpp)
	execute_process(COMMAND  thrift -r --gen cpp 
							 -o ${CMAKE_CURRENT_SOURCE_DIR}/thriftgen-${THRIFT_VERSION}/
							 ${CMAKE_CURRENT_SOURCE_DIR}/Broker.thrift)		 
endif ()

# set current gen-cpp/ included path				 
execute_process(COMMAND rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp)	
execute_process(COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/thriftgen-${THRIFT_VERSION}/gen-cpp
	                            ${CMAKE_CURRENT_SOURCE_DIR}/)	

# libswcdb_thrift
ADD_LIB_TARGET(
	NAME 	swcdb_thrift
	SRCS 	gen-cpp/Service_constants.cpp 
				gen-cpp/Service_types.cpp
				gen-cpp/Service.cpp 
				# Converters.cc Client.cc
	SHARED ${THRIFT_LIBRARIES_SHARED}
	STATIC ${THRIFT_LIBRARIES_STATIC}
)

endif()
