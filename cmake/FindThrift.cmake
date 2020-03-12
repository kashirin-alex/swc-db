#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#


SET_DEPS(NAME "EVENT" REQUIRED TRUE LIB_PATHS "" INC_PATHS "" STATIC libevent.a SHARED event INCLUDE event.h)

	 
SET_DEPS(
	NAME "THRIFT" 
	REQUIRED TRUE 
	LIB_PATHS /usr/local/lib
	INC_PATHS /usr/local/include
			      /usr/include
			      /opt/local/include
	STATIC libthrift.a libthriftnb.a  libthriftz.a 
	SHARED thrift thriftnb thriftz
	INCLUDE thrift/Thrift.h
)


exec_program(env ARGS thrift -version OUTPUT_VARIABLE THRIFT_VERSION
  	          RETURN_VALUE Thrift_RETURN)
if(THRIFT_VERSION MATCHES "^Thrift version" AND THRIFT_FOUND AND EVENT_FOUND) 
	set(THRIFT_LIBRARIES_SHARED ${THRIFT_LIBRARIES_SHARED} ${EVENT_LIBRARIES_SHARED})
	set(THRIFT_LIBRARIES_STATIC ${THRIFT_LIBRARIES_STATIC} ${EVENT_LIBRARIES_STATIC})
	
  message("       compiler: ${THRIFT_VERSION}")
  string(REPLACE "\n" " " THRIFT_VERSION ${THRIFT_VERSION})
  string(REPLACE " " ";" THRIFT_VERSION ${THRIFT_VERSION})
  list(GET THRIFT_VERSION -1 THRIFT_VERSION)

else()
	set(THRIFT_FOUND FALSE)
	
endif()