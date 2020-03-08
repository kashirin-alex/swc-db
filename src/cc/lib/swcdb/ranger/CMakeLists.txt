#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#

set(swcdb_librs_sources
)



# lib Ranger
#ADD_LIB_TARGET(
#	NAME Ranger 
#	SRCS ${swcdb_librs_sources}
#	TARGETS swcdb_db
#)

	

ADD_TEST_TARGET(
	NAME	cellstore
	SRCS	db/tests/test_cellstore.cc
	TARGETS	swcdb_core swcdb_core_config swcdb_core_comm swcdb_core_sys swcdb_fs swcdb_client ${BUILTIN_FS_TARGETS}
	ARGS	 "--swc.fs=local" "--swc.rgr.ram.percent=2"

	SHARED  ${SNAPPY_LIBRARIES_SHARED}
	STATIC  ${SNAPPY_LIBRARIES_STATIC} 
)

ADD_TEST_TARGET(
	NAME	commitlog
	SRCS	db/tests/test_commitlog.cc
	TARGETS	swcdb_core swcdb_core_config swcdb_core_comm swcdb_core_sys swcdb_fs swcdb_client ${BUILTIN_FS_TARGETS}
	ARGS 	"--swc.fs=local" "--swc.rgr.ram.percent=2"

	SHARED  ${SNAPPY_LIBRARIES_SHARED}
	STATIC  ${SNAPPY_LIBRARIES_STATIC} 
)
