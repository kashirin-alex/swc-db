#
# Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
#

set(swcdb_sources
	Cells/Vector.cc
	Protocol/Rgr/params/Report.cc
	Protocol/Rgr/req/Report.cc
	client/Settings.cc
	client/Clients.cc
	client/Schemas.cc
	client/AppContext.cc
	client/rgr/Rangers.cc
	client/mngr/Groups.cc
	client/Query/Update.cc
	client/Query/Select.cc
	client/sql/SQL.cc
	client/sql/Reader.cc
	client/sql/ColumnList.cc
	client/sql/ColumnSchema.cc
	client/sql/QuerySelect.cc
	client/sql/QueryUpdate.cc
)
ADD_LIB_TARGET(
	NAME 		swcdb
	SRCS 		${swcdb_sources}
	TARGETS swcdb_core swcdb_core_config swcdb_core_comm
)



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
	TARGETS	swcdb_core swcdb_core_config swcdb
)



ADD_TEST_TARGET(
	NAME      test_echo
	SRCS      client/test/test_echo.cc
	TARGETS   swcdb_core swcdb_core_config swcdb_core_comm swcdb
)

ADD_TEST_TARGET(
	NAME      test_mngr_client
	SRCS      client/test/test_mngr_client.cc
	TARGETS   swcdb_core swcdb_core_config swcdb_core_comm swcdb
)

ADD_TEST_TARGET(
	NAME      test_mngr_mng_column
	SRCS      client/test/test_mngr_mng_column.cc
	TARGETS   swcdb_core swcdb_core_config swcdb_core_comm swcdb
)
ADD_TEST_TARGET(
	NAME      test_query
	SRCS      client/test/test_query.cc
	TARGETS   swcdb_core swcdb_core_config swcdb_core_comm swcdb
)