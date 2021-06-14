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
  Types/SystemColumn.cc
  Types/MngrRole.cc
  Types/MngrColumnState.cc
  Types/MngrRangeState.cc
  Types/MngrRangerState.cc
  Types/MngrState.cc

  Protocol/Commands.cc
  Protocol/Common/params/HostEndPoints.cc

  Protocol/Rgr/params/Report.cc
  Protocol/Rgr/req/Report.cc
  Protocol/Rgr/params/ColumnCompact.cc
  Protocol/Rgr/params/RangeLocate.cc
  Protocol/Rgr/params/RangeQueryUpdate.cc
  Protocol/Rgr/params/RangeQuerySelect.cc

  Protocol/Mngr/params/MngrActive.cc
  Protocol/Mngr/req/MngrActive.cc
  Protocol/Mngr/params/RgrGet.cc
  Protocol/Mngr/req/RgrGet_Base.cc
  Protocol/Mngr/params/ColumnCompact.cc
  Protocol/Mngr/req/ColumnCompact_Base.cc
  Protocol/Mngr/params/ColumnGet.cc
  Protocol/Mngr/req/ColumnGet_Base.cc
  Protocol/Mngr/params/ColumnList.cc
  Protocol/Mngr/req/ColumnList_Base.cc
  Protocol/Mngr/params/ColumnMng.cc
  Protocol/Mngr/req/ColumnMng_Base.cc
  Protocol/Mngr/params/Report.cc
  Protocol/Mngr/req/Report.cc

  Protocol/Bkr/params/CellsUpdate.cc
  Protocol/Bkr/params/CellsSelect.cc
  Protocol/Bkr/req/ColumnCompact_Base.cc
  Protocol/Bkr/req/ColumnGet_Base.cc
  Protocol/Bkr/req/ColumnList_Base.cc
  Protocol/Bkr/req/ColumnMng_Base.cc

  client/Settings.cc
  client/Clients.cc
  client/Schemas.cc

  client/service/mngr/ContextManager.cc
  client/service/mngr/Groups.cc
  client/service/mngr/Managers.cc

  client/service/rgr/ContextRanger.cc
  client/service/rgr/Cache.cc
  client/service/rgr/Rangers.cc

  client/service/bkr/ContextBroker.cc
  client/service/bkr/Brokers.cc

  client/Query/Update/Handlers/Base.cc
  client/Query/Update/Handlers/BaseColumnMutable.cc
  client/Query/Update/Handlers/BaseUnorderedMap.cc
  client/Query/Update/Handlers/Common.cc
  client/Query/Update/Committer.cc
  client/Query/Update/BrokerCommitter.cc
  client/Query/Update/Handlers/Metrics.cc

  client/Query/Select/Handlers/Base.cc
  client/Query/Select/Handlers/BaseUnorderedMap.cc
  client/Query/Select/Handlers/BaseSingleColumn.cc
  client/Query/Select/Handlers/Common.cc
  client/Query/Select/Scanner.cc
  client/Query/Select/BrokerScanner.cc

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
