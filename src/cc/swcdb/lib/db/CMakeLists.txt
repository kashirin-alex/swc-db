#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#

set(swcdb_db_sources
	Comparators/Comparators.cc
	ScanSpecs/ScanSpecs.cc
)


ADD_LIB_TARGET(
	NAME 		swcdb_db
	SRCS 		${swcdb_db_sources}
	TARGETS swcdb_core
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

ADD_TEST_TARGET(
	NAME      columns
	SRCS      Columns/tests/test_columns.cc
	TARGETS 	swcdb_core_comm swcdb_core_config swcdb_db  ${BUILTIN_FS_TARGETS}
)
		