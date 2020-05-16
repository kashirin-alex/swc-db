#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
#

set(swcdb_sources
	Cells/Mutable.cc
	Cells/Result.cc
	Cells/MapMutable.cc
	Cells/Interval.cc
	Cells/CellKeyVec.cc
	Cells/CellKey.cc
	Cells/Cell.cc
	Cells/KeyComparator.cc

	Cells/SpecsScan.cc
	Cells/SpecsColumn.cc
	Cells/SpecsInterval.cc
	Cells/SpecsKey.cc
	Cells/SpecsTimestamp.cc
	Cells/SpecsValue.cc
	Cells/SpecsFlags.cc
	
	Cells/ReqScan.cc
	
	Columns/Schema.cc
	Columns/Schemas.cc

	Types/Column.cc
	Types/Encoding.cc
	Types/Range.cc
	Types/KeySeq.cc
	Types/MetaColumn.cc

	Protocol/Common/params/HostEndPoints.cc

	Protocol/Rgr/params/Report.cc
	Protocol/Rgr/req/Report.cc
	Protocol/Rgr/params/ColumnCompact.cc
	Protocol/Rgr/req/ColumnCompact.cc
	Protocol/Rgr/params/RangeLocate.cc
	Protocol/Rgr/req/RangeLocate.cc
	Protocol/Rgr/params/RangeQueryUpdate.cc
	Protocol/Rgr/req/RangeQueryUpdate.cc
	Protocol/Rgr/params/RangeQuerySelect.cc
	Protocol/Rgr/req/RangeQuerySelect.cc
	
	Protocol/Mngr/params/MngrActive.cc
	Protocol/Mngr/req/MngrActive.cc
	Protocol/Mngr/params/RgrGet.cc
	Protocol/Mngr/req/RgrGet.cc
	Protocol/Mngr/params/ColumnCompact.cc
	Protocol/Mngr/req/ColumnCompact.cc
	Protocol/Mngr/params/ColumnGet.cc
	Protocol/Mngr/req/ColumnGet.cc
	Protocol/Mngr/params/ColumnList.cc
	Protocol/Mngr/req/ColumnList.cc
	Protocol/Mngr/params/ColumnMng.cc
	Protocol/Mngr/req/ColumnMng.cc
	
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
	NAME	cell_key
	SRCS	Cells/tests/test_cell_key.cc
	TARGETS	swcdb_core swcdb
)
ADD_TEST_TARGET(
	NAME	cell
	SRCS	Cells/tests/test_cell.cc
	#TARGETS	swcdb_core swcdb
	FLAGS "-DSWC_IMPL_SOURCE"
)

ADD_TEST_TARGET(
	NAME	cell_specs_scan
	SRCS	Cells/tests/test_cell_specs_scan.cc
	TARGETS	swcdb_core swcdb
)
ADD_TEST_TARGET(
	NAME	cells_interval
	SRCS	Cells/tests/test_cells_interval.cc
	TARGETS	swcdb_core swcdb
)

ADD_TEST_TARGET(
	NAME	cells_mutable
	SRCS	Cells/tests/test_cells_mutable.cc
	TARGETS	swcdb_core swcdb_core_config swcdb
)

ADD_TEST_TARGET(
	NAME	key_comparator
	SRCS	Cells/tests/test_key_comparator.cc
	TARGETS	swcdb_core swcdb_core_config swcdb
	#FLAGS "-DSWC_IMPL_SOURCE"
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