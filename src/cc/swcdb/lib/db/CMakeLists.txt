#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#

set(swcdb_db_sources
	Comparators/Comparators.cc
)


ADD_LIB_TARGET(
	NAME 		swcdb_db
	SRCS 		${swcdb_db_sources}
	TARGETS swcdb_common
)

ADD_TEST_TARGET(
	NAME      comparators
	SRCS      Comparators/tests/test_comparators.cc
	TARGETS 	swcdb_db
)


ADD_TEST_TARGET(
	NAME      scanspecs
	SRCS      ScanSpecs/tests/test_scanspecs.cc
	TARGETS 	swcdb_db
)
	

ADD_TEST_TARGET(
	NAME    cell
	SRCS    Cells/tests/test_cell.cc
	TARGETS swcdb_db
)

		