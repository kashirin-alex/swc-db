#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>



set(swcdb_sources
  ${SWC_VERSION_SRC}

  Cells/Mutable.cc
  Cells/Result.cc
  Cells/MutableVec.cc

  Cells/Interval.cc
  Cells/CellKeyVec.cc
  Cells/CellKey.cc
  Cells/CellValueSerialFields.cc
  Cells/Cell.cc
  Cells/KeyComparator.cc

  Cells/SpecsScan.cc
  Cells/SpecsColumn.cc
  Cells/SpecsInterval.cc
  Cells/SpecsKeyIntervals.cc
  Cells/SpecsKey.cc
  Cells/SpecsTimestamp.cc
  Cells/SpecsValues.cc
  Cells/SpecsValue.cc
  Cells/SpecsValueSerialFields.cc
  Cells/SpecsFlags.cc

  Cells/ReqScan.cc

  Columns/Schema.cc
  Columns/Schemas.cc

  Types/Column.cc
  Types/Range.cc
  Types/KeySeq.cc
  Types/MetaColumn.cc
  Types/MngrRole.cc
  Types/MngrColumnState.cc
  Types/MngrRangeState.cc
  Types/MngrRangerState.cc
  Types/MngrState.cc

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
  Protocol/Mngr/params/Report.cc
  Protocol/Mngr/req/Report.cc

  client/Settings.cc
  client/Clients.cc
  client/Schemas.cc
  client/ContextManager.cc
  client/ContextRanger.cc
  client/rgr/Rangers.cc
  client/mngr/Groups.cc
  client/Query/UpdateHandlerBaseColumnMutable.cc
  client/Query/UpdateHandlerBaseUnorderedMap.cc
  client/Query/UpdateHandlerCommon.cc
  client/Query/Update.cc
  client/Query/SelectHandlerBaseUnorderedMap.cc
  client/Query/SelectHandlerBaseSingleColumn.cc
  client/Query/SelectHandlerCommon.cc
  client/Query/Select.cc
  client/sql/SQL.cc
  client/sql/Reader.cc
  client/sql/ColumnList.cc
  client/sql/ColumnSchema.cc
  client/sql/QuerySelect.cc
  client/sql/QueryUpdate.cc
)

ADD_LIB_TARGET(
  NAME      swcdb
  SRCS      ${swcdb_sources}
  TARGETS   ${SWC_LIB_CORE_TARGET}
            ${SWC_LIB_CORE_CONFIG_TARGET}
            ${SWC_LIB_CORE_COMM_TARGET}
  SHARED    ${SWC_LIB_CORE_LIBRARIES_SHARED}
            ${SWC_LIB_CORE_CONFIG_LIBRARIES_SHARED}
            ${SWC_LIB_CORE_COMM_LIBRARIES_SHARED}
  STATIC    ${SWC_LIB_CORE_LIBRARIES_STATIC}
            ${SWC_LIB_CORE_CONFIG_LIBRARIES_STATIC}
            ${SWC_LIB_CORE_COMM_LIBRARIES_STATIC}
)
