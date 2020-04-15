#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#



exec_program(env ARGS thrift -version OUTPUT_VARIABLE THRIFT_VERSION
  	          RETURN_VALUE Thrift_RETURN)

if(THRIFT_VERSION MATCHES "^Thrift version") 
	set(THRIFT_COMPILER_FOUND TRUE)
  message("-- Found THRIFT compiler")
  message("       compiler: ${THRIFT_VERSION}")
  string(REPLACE "\n" " " THRIFT_VERSION ${THRIFT_VERSION})
  string(REPLACE " " ";" THRIFT_VERSION ${THRIFT_VERSION})
  list(GET THRIFT_VERSION -1 THRIFT_VERSION)

else()
	set(THRIFT_COMPILER_FOUND FALSE)
  message("-- Not Found THRIFT compiler: thrift")
  message("       Using existing gen. sources")
endif()
	 


SET_DEPS(NAME "EVENT" REQUIRED TRUE LIB_PATHS "" INC_PATHS "" STATIC libevent.a SHARED event INCLUDE event.h)
if(EVENT_FOUND)
SET_DEPS(
	NAME "THRIFT_CPP" 
	REQUIRED FALSE 
	LIB_PATHS /usr/local/lib
	INC_PATHS /usr/local/include
			      /usr/include
			      /opt/local/include
	STATIC libthrift.a libthriftnb.a  libthriftz.a 
	SHARED thrift thriftnb thriftz
	INCLUDE thrift/Thrift.h
)
set(THRIFT_CPP_LIBRARIES_SHARED ${THRIFT_CPP_LIBRARIES_SHARED} ${EVENT_LIBRARIES_SHARED})
set(THRIFT_CPP_LIBRARIES_STATIC ${THRIFT_CPP_LIBRARIES_STATIC} ${EVENT_LIBRARIES_STATIC})
endif()


SET_DEPS(
	NAME "GLIB" REQUIRED TRUE 
	LIB_PATHS ""
	INC_PATHS ""
	STATIC ""
	SHARED glib-2.0 gobject-2.0
	INCLUDE glib-2.0/glib-object.h
)
if(GLIB_FOUND)
SET_DEPS(
	NAME "THRIFT_C" 
	REQUIRED FALSE 
	LIB_PATHS /usr/local/lib
	INC_PATHS /usr/local/include
			      /usr/include
			      /opt/local/include
	STATIC libthrift_c_glib.a
	SHARED thrift_c_glib
	INCLUDE thrift/c_glib/thrift.h
)
set(THRIFT_C_LIBRARIES_SHARED ${THRIFT_C_LIBRARIES_SHARED} ${GLIB_LIBRARIES_SHARED})
set(THRIFT_C_LIBRARIES_STATIC ${THRIFT_C_LIBRARIES_STATIC} ${GLIB_LIBRARIES_STATIC})

endif()