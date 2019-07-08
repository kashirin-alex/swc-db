#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#

set(swcdb_librs_sources
)


ADD_TEST_TARGET(
	NAME      columns
	SRCS      columns/tests/test_columns.cc
	TARGETS 	swcdb_db
)

# lib RangeServer
#ADD_LIB_TARGET(
#	NAME RangeServer 
#	SRCS ${swcdb_librs_sources}
#	TARGETS swcdb_db
#)
