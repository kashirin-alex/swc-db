#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#


ADD_TEST_TARGET(
	NAME	comparators
	SRCS	Cells/tests/test_comparators.cc
	TARGETS	swcdb_core
)
ADD_TEST_TARGET(
	NAME	cell_key
	SRCS	Cells/tests/test_cell_key.cc
	TARGETS	swcdb_core
)
ADD_TEST_TARGET(
	NAME	cell
	SRCS	Cells/tests/test_cell.cc
	TARGETS	swcdb_core
)

ADD_TEST_TARGET(
	NAME	cell_specs_scan
	SRCS	Cells/tests/test_cell_specs_scan.cc
	TARGETS	swcdb_core
)
ADD_TEST_TARGET(
	NAME	cells_interval
	SRCS	Cells/tests/test_cells_interval.cc
	TARGETS	swcdb_core
)

ADD_TEST_TARGET(
	NAME	cells_mutable
	SRCS	Cells/tests/test_cells_mutable.cc
	TARGETS	swcdb_core
)
	

ADD_TEST_TARGET(
	NAME	columns
	SRCS	Columns/tests/test_columns.cc
	TARGETS	swcdb_core_config swcdb_core ${BUILTIN_FS_TARGETS}
	ARGS	"--config=/root/swc-db/conf/swc.cfg" "--swc.fs=local"
)

