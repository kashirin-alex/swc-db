#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#

# set(ht_src_path ${CMAKE_CURRENT_SOURCE_DIR}/../../../hypertable/Common)

set(swcdb_common_sources
	String.cc
	Error.cc
	Logger.cc
	InetAddr.cc
	Serializable.cc
)


ADD_LIB_TARGET(
	NAME 		swcdb_common 
	SRCS 		${swcdb_common_sources}
	SHARED  ${MALLOC_LIBRARIES_SHARED} ${RE2_LIBRARIES_SHARED}
	STATIC  ${MALLOC_LIBRARIES_STATIC} ${RE2_LIBRARIES_STATIC}
)
