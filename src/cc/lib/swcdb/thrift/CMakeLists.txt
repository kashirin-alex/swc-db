#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#



if(THRIFT_CPP_GEN_PATH)

# libswcdb_thrift
ADD_LIB_TARGET(
	NAME 	swcdb_thrift
	SRCS 	${THRIFT_CPP_GEN_PATH}/gen-cpp/Service_constants.cpp 
        ${THRIFT_CPP_GEN_PATH}/gen-cpp/Service_types.cpp
        ${THRIFT_CPP_GEN_PATH}/gen-cpp/Service.cpp 
        # Converters.cc 
        # Client.cc
	SHARED ${THRIFT_CPP_LIBRARIES_SHARED}
	STATIC ${THRIFT_CPP_LIBRARIES_STATIC}
)

ADD_TEST_TARGET(
	NAME thrift_client
	SRCS client/tests/client.cc
	TARGETS swcdb_thrift 
)


endif()
