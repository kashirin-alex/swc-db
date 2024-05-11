# **SWC-DB©** - History/Changes Log
**All notable SWC-DB© changes and releases are documented in this file.** _- start 'watching' the master branch to receive updates_



***




### [SWC-DB master](https://github.com/kashirin-alex/swc-db/tree/master) (upcoming-release)

    added Scan Specification of Serial Value with an 'OR' condition support
    added unit8_t Condition::COMP_EXTENDED_DOMAIN = 0x04
    added Condition::Comp::OR = 0x15
    added DB::Specs::Serial::Value::Fields ctor option 'with_state'
    added SQL parsing support for ||(Condition::OR) in select command
    added bool DB::Specs::Serial::Value::Fields::is_matching_or(const Cell&)
    updated for openssl-3.2.1 zlib-1.3 thrift-0.20.0 asio-1.30.2 zstd-1.5.6
    Resolves Issue: #14

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.5.12...master)
******




### [SWC-DB v0.5.12](https://github.com/kashirin-alex/swc-db/releases/tag/v0.5.12) (2023-06-03)

    fixed early Core::Comm::server::Acceptor Accept before Init completion
    moved DB::Cells::Interval::align(KeyVec&,KeyVec&) to source file
    removed include swcdb/core/Compat.h from Version.h
    added class SWC::Memory with static functions 'allocate' and 'free'
    added swcdb/core/Malloc.cc by default impl. by Malloc.h
    removed from functions a default bool value cases for 'own' arg
    added Value::Field_LIST_{INT64,BYTES}::write<T>(DynamicBuffer*, const T&)
    added '-ffunction-sections -fdata-sections' compiler flags for Releases
    added '-Wl,--gc-sections' linker flags for Releases
    fixed invalid use of update handler iterator in ThriftBroker updater_close
    added warning flag '-Wdeprecated-copy-dtor'
    changed compatibility for '-Weffc++'
    added noexcept Core::LogWriter ctor and marked copy/move 'delete'
    changed Core::Exception to single ctor and Args to only moveable
    fixed Manager state change of Rangers at rgr_report
    fixed Manager Rangers state sync at a run of RANGERS and COLUMNS roles
    changed Python logging in swcdb.thrift.pool to quieter mood
    Resolves Issue: #13
    added cases of user-defined ctors & operators instead of default or delete
    added uint32_t Comm:IoContext::get_number_of_threads(bool, int32_t)
    added Comm:IoContextPtr Comm:IoContext::make(std::string&&, bool, int32_t)
    added cfg swc.{bkr,mngr,rgr,FsBroker}.concurrency.relative BOOL
    added cfg swc.fs.{broker,hadoop}.concurrency.relative BOOL
    added Comm::server::SerializedServer(.. bool concurrency_relative)
    added relative HW-Concurrency based Thread Pools to Services supported
    added Ranger swc.rgr.loader.handlers INT32 configuration property
    changed Ranger BlockLoader load acknowledgement with condition_variable
    fixed use-after-free possibility of class Ranger::BlockLoader pointer
    added Utils::shell::Interface at errno EINTR skip break
    changed FS::Callback::*Read* Cb_t from StaticBuffer::Ptr to StaticBuffer&&
    added StaticBuffer&& argument to SWC::FS::Callback::WriteCb_t callback
    changed bool ResponseCallback::expired(unit64_t) const noexcept to virtual
    added bool Ranger::CompactRange::expired(unit64_t) const noexcept override
    updated for asio-1.28.0 openssl-3.1.0 zstd-1.5.5 thrift-0.18.1

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.5.11...v0.5.12)
******




### [SWC-DB v0.5.11](https://github.com/kashirin-alex/swc-db/releases/tag/v0.5.11) (2023-01-14)

    added Encoder support for Plain and Serial value in update Cells Interval
    added Thrift Service specialized methods and dedicated Structs
    Thrift Service related changes:
        changed CellValueSerial/Op fields value to optional
        changed 'columns' in SpecScan to columns_{plain,counter}
        renamed struct ColCells to cCells
        renamed struct Cell to CellPlain
        renamed struct UCell to UCellPlain
        added struct {FCell,KCell,CCell}{Plain,Counter} 
        removed struct FCell, KCell, CCell
        added struct SpecInterval{Plain,Counter}
        added struct SpecIntervalUpdate{Plain,Counter}
        added struct SpecValue{Plain,Counter}
        added struct SpecColumn{Plain,Counter}
        removed struct Spec{Column,Value,IntervalUpdate,Interval}
        added typedef list<CellSerial>  CellsSerial
        added typedef list<CellCounter> CellsCounter
        added typedef list<CellPlain>   CellsPlain
        added typedef list<UCellCounter> UCellsCounter
        added typedef list<UCellPlain> UCellsPlain
        added typedef map<i64, UCellsCounter> UCCellsCounter
        added typedef map<i64, UCellsPlain> UCCellsPlain
        added const i8 COUNTER_OP_EQUAL = 1
        added 'counter_cells' to struct Cells
        renamed 'cells' in struct Cells to plain_cells
        added '{plain,counter}_cells' to struct {FCells,kCells,cCells}
        added 'counter' type to method update_by_types
        added method update_counter
        changed method update to update_plain
        added method CellsPlain sql_select_plain(..)
        added method CellsCounter sql_select_counter(..)
        added method CellsSerial sql_select_serial(..)
        added col-type Mismatch Exception at ThriftBroker::AppHandler::sync_select
        changed Thrift::Converter::SpecInterval to SpecInterval{Plain,Counter}
    added Encoder DB::Specs::IntervalUpdate::encoder
    added client::SQL::Reader::Reader( .., DB::Types::Encoder*, ..)
    changed DB::Cells::Cell::get_value(&, bool) return DB::Types::Encoder
    added Ranger RangeQuerySelectUpdating*::update_cell_value by Encoder
    resolved issue #12

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.5.10...v0.5.11)
******




### [SWC-DB v0.5.10](https://github.com/kashirin-alex/swc-db/releases/tag/v0.5.10) (2022-10-01)

    added Thrift Service method 'update_by_types' update with one call
    added Thrift Service constants TIMESTAMP_NULL & TIMESTAMP_AUTO
    added Condition::Comp::{FIP,FI} fraction include {prior}
    added consts uint8_t Condition::COMP_EXTENDED_{KEY,VALUE}
    added enum Thrift::Comp::{FIP,FI} fraction include {prior}
    changed use case of Condition::GE "" to Condition::FIP
    added SQL support for FI/P condition literals ":<"(FIP) & ":"(FI)
    fixed ambiguous -GE/T(">=") condition purposing for inner-fraction scan
    resolved issue #11
    changed TSV::FileReader::clients to Update::Handlers::Common::Ptr hdlr
    fixed requirement of intialized update-handler in client::SQL::parse_load
    fixed 'load' command in shell SWC-DB(client)>
    fixed Utils::shell::Interface::run() early CLI::QUIT_CLI at errno EAGAIN
    added shell::CLI state argument to Utils::shell::Interface constructor
    changed default to -flto=1
    fixed numbers begin with zero Core::Time::parse_ns(..)
    added Core::Time::parse_ns(..) support for date-format in short-form
    updated for thrift-0.17.0 asio-1.24.0 openssl-3.0.5

    notes:
        * older releases are incompatible with
          the Condition-Key select SQL syntax and the DB::Specs::Key
          of an inner-level fractions select by the GT/E empty-string
          requires a change of comparators from -GT to -FI and -GE to -FIP

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.5.9...v0.5.10)
******




### [SWC-DB v0.5.9](https://github.com/kashirin-alex/swc-db/releases/tag/v0.5.9) (2022-05-21)

    fixed missing read_history at non-replxx in Utils::shell::Interface
    fixed Utils::shell::Interface at CTRL+C exit gracefully
    added support of update-operation in Cells-Interval select-query Specs
    added struct UpdateOP file swcdb/db/Cells/SpecsUpdateOp.h
    added UpdateOP DB::Specs::IntervalUpdate::operation
    added base/interface class DB::Cell::Serial::Value::FieldUpdate
    added classes DB::Cell::Serial::Value::FieldUpdate_*TYPE
    added files db/cells/Field{s,Update} for CellValueSerialFields.{h,cc}
    added void DB::Cell::Serial::Value::FieldsWriter::add(FieldUpdate*)
    added class DB::Cell::Serial::Value::FieldsUpdaterMap
    added client::SQL::Reader::read_operation(col_type, UpdateOP&)
    fixed Ranger ReqScan with an update require a value at ONLY_KEYS
    added Ranger RangeQuerySelectUpdating_{Ap/Prepend,Insert,Overwrite,Serial}
    added Ranger use Select-callback by interval.updating->operation.op
    changed Ranger RangeQuerySelectDeleting DELETE_LE flag at cell-versions==1
    added query-select SQL syntax and Thrift Service support for Update OP
    added python module 'serialization' with methods decode_{vi64,counter}(..)
    updated for ASIO 1.22.1 and OPENSSL_NO_DEPRECATED=ON by default

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.5.8...v0.5.9)
******




### [SWC-DB v0.5.8](https://github.com/kashirin-alex/swc-db/releases/tag/v0.5.8) (2022-02-26)

    added options representations in Specs::Interval::{print,display}
    changed Manager::MngdColumns::remove to use DELETE_MATCHING in Intval-spec
    changed Utils::shell::Statistics::truncate with Intval-spec DELETE_MATCHING
    added support for 'replxx' (if FOUND, the library defaults)
    changed LogWriter::_seconds() to _time_and_level(..)
    changed Logger's log functions & make Exception(..) scopes to 'noexcept'
    added example of queue for C++ Client/API
    updated for Apache-Thrift v0.16.0

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.5.7...v0.5.8)
******




### [SWC-DB v0.5.7](https://github.com/kashirin-alex/swc-db/releases/tag/v0.5.7) (2022-01-23)

    updated for OpenSSL 3 version
    added cfg-files swc_fs_{ceph,hadoop_jvm,local}.dyn.cfg
    added cfg 'swc.fs.hadoop_jvm.{read,write}.buffer.size'
    added cfg 'swc.fs.hadoop_jvm.block.size'
    removed 'bufsz' arg from FS::FileSystem::{open,create}
    removed 'blksz' arg from FS::FileSystem::{create,write)
    changed FileSystemHadoopJVM::{open,create} to use cfg_{w,r}_buffer_size
    changed FileSystemCeph::create to use cfg-values at ceph_open_layout
    added struct FS::ImplOptions and FS::FileSystem::impl_options
    added (dev-stage) -DSWC_FS_LOCAL_USE_IO_URING
    added FileSystemLocal::read(Callback::ReadAllCb_t&&, const std::string&)
    added async option in FsBroker::Handler::read_all
    removed FS::Callback::* args SmartFd::Ptr and std::string
    added move-ctor for Core::{QueuePointer, QueueSafe}
    fixed Ranger mem-usage of Range::(m_q_scan,m_q_add) clear-expired requests
    fixed Ranger CompactRange::initialize wait-time + fragments size * 100ms
    removed Ranger Fragments grouping of one-version without filling-check
    fixed Ranger CellStores need-compact of multi-versions over cellstores
    changed FsBrokers handlers to use FileSystem's default available methods
    added ensure fd-closed in FileSystem::default_{write,read,combi_pread}
    fixed Manager hint of a last-cid-used
    changed SWC_PRINT logger Macro to use unique var-name for mutex-lock
    removed Cells::{AUTO_TIMESTAMP,AUTO_ASSIGN}, incompatible to old-releases
    changed Cell::{timestamp,revision} to private ctx & added set/get support
    fixed Cell::print of COUNTER type with flag DELETE
    added DB::Cells::Cell 'control' MASK bits
    fixed missing VectorsVector<>:: ctor & assign for copy & move
    added bool VectorsVector<>::Const/Iterator::next()
    changed ctors of VectorsVector<>::Const/Iterator(VectorsT&) to explicit
    added ensure free() before DB::Cells::Mutable move
    added Cells::Mutable::_add_plain_version_{single,multi}(..)
    changed Mutable::split to split(Mutable&, size_t, size_t, bool)
    fixed cells split of the same key in Cells::MutableVec
    fixed offset-state at 'add_sorted' in Cells::MutableVec::add_raw
    added support of unfinalized Cells::Mutable
    fixed and extended revisions-control of Counter Column types
    added {Mutable/Vec,Fragments}::check_sequence(const char*, bool w_assert)
    added a Python Example for load_generator sequence-check
    added void SQL::Reader::read_ts_and_value(..) with opt. TS-TOKEN "auto"
    moved SQL::Reader::counter_op_from to static int Cell::counter_from_str(.)
    added Thrift::Converter::set(const CellValuesSerial&,Value::FieldsWriter&)
    added virtual void DB::Cells::ReqScan::update(Mutable& cells)
    added virtual bool DB::Cells::ReqScan::has_update()
    added Query support for an Update and a Delete cell/s on Scan Specs match
    added DELETE_MATCHING and UPDATE=(TS,DATA,ENC) Cells-Interval SQL support
    added Thrift Service support for Specs-Interval-Update (update & delete)
    changed Comm::ConnHandler::{Sender,Receiver}_*::operator()(..) to noexcept

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.5.6...v0.5.7)
******




### [SWC-DB v0.5.6](https://github.com/kashirin-alex/swc-db/releases/tag/v0.5.6) (2021-10-25)

    added applicable function attributes const and pure
    added Utilities Shell Interface::add_option & moved Option to private
    added -fdevirtualize-at-ltrans with -flto optimization builds
    fixed Endpoints re-use in Query Update::Committer locate_on_ranger ReqData
    added cmake option -DUSE_LIBSSL=open/wolf and support with libwolfssl
    changed ConnHandlerSSL::handshake & set_verify to STL by Handler
    added ConnHandlerSSL::Ptr ConfigSSL::make_connection(..)
    removed ConfigSSL::make_client/server
    removed class Comm::client::ServerConnections
    removed 'preserve' option in client::Serialized
    changed Sync client::Serialized::get_connection to single function
    added (private sub-class) client::Serialized::Connector<CbT>
    changed Async Serialized::get_connection to STL and rm use of lambda
    added open-connections management in FileSystemBroker
    changed Manager MngrRole::managers_checker CB-handler to struct
    changed Comm::client::Host::connect() CB-handler to struct
    added const char* Core::type_name(const T& obj) & added type-name logging
    added bool Env::Mngr::is_shuttingdown()
    fixed Manager shutting-down while at Range::get_last_rgr
    added macros SWC_FMT_LU/LD, SWC_QUICK_EXIT
    added a static function typedef Config::Setting::init_option_t
    changed Config::Setting 'init' interface with func* init_app/post_cmd_args
    changed applications Settings functions defition to run with init_option_t
    changed '*.probes' Config Properties to G_UINT16
    renamed classes Config V_* to Value_* and enum Value::Type::* to TYPE_*
    changed DB::Cells::Flag DELETE to DELETE_LE & DELETE_VERSION to DELETE_EQ
    changed Comm Protocol::Mngr::Params::ColumnMng::Function::DELETE to REMOVE
    updated SWC::Thrift protocol to changes in SWC::Comm Protocol::
    fixed use of keywords 'DELETE' & 'BOOL' for cross-platform/compilers
    changed Config::Settings to use std::filesystem::remove instead ::unlink
    renamed cmake option USE_GLIBC_MALLOC to USE_DEFAULT_MALLOC
    changed strptime to specialized parser in Core::Time::parse_ns(..)
    added Experimental(at non-production) support for msys2-MingW64
    added Manager create Ranges-Dir in Column::init (halted/bad remove/create)
    fixed weak-iteration in Manager::Columns::assigned(rgr_t, ..)
    added separate cmake Flags files cmake/Flags{Build,Warnings,Profile}.cmake
    removed nullptr pointer checks before ::operator::delete/[]
    changed stack-usage reductions to down-under 1024-bytes
    added Ranger cfg-property 'swc.rgr.compaction.range.uncompacted.max'
    added ThriftBroker AppHandler Processing state and wait-completion at stop
    updated for ASIO-1.20.0

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.5.5...v0.5.6)
******




### [SWC-DB v0.5.5](https://github.com/kashirin-alex/swc-db/releases/tag/v0.5.5) (2021-09-21)

    added Config::Property::V_GUINT64 (G_UINT64)
    added class Manager::Schemas derived from DB::Schemas
    changed Env::Mngr::m_schemas to Manager::Schemas
    added Manager Configuration Properties:
      G_UINT64 swc.mngr.schemas.store.from.capacity
      G_INT32  swc.mngr.schemas.store.block.size
      G_ENUM   swc.mngr.schemas.store.block.encoder
    added Manager at shutdown save-schemas and fast-load from Schemas-Store
    changed sbin/swcdb_cluster kill/stop exec-seq. wait-ready after send-kill
    added class Core::StateSynchronization, replaces use cases of std::promise
    added bool Comm::equal_endpoints(const EndPoints& , const EndPoints&)
    changed fs-block-size without specific to default-0, FS dependable/default
    added definer SWC_RANGER_WITH_RANGEDATA default OFF "range.data" hint-file
    added RgrData storage in Column SYS_RGR_DATA for CID above 9
    added system reserved column SYS_RGR_DATA (cid=9) other moved up +1
    added DB::Types::SystemColumn::is_rgr_data_on_fs(cid_t cid)
    added class DB::RgrData (handles SYS_RGR_DATA)
    changed Common::Files::RgrData Cls to NS & functions accepts DB::RgrData&
    changed Manager::Range::m_last_rgr to std::unique_ptr<DB::RgrData>
    added auto-create SYS_RGR_DATA in Common::Files::Schema::load
    added Manager::Columns pre-load SYS_RGR_DATA after Master & Meta Ranges
    added unload-phase for cols in SYS_RGR_DATA Ranger::Columns::unload_all
    added delete all RgrData at colm-delete in Manager::MngdColumns::remove
    changed Env::Rgr::m_rgr_data to DB::RgrData
    removed Ranger Range::get_last_rgr(int &err)
    added Ranger::Range::{set,remove}_rgr(int& err)
    added #include <unordered_map> and <map> to Compat.h
    fixed Error::get_text access with operator[] (unsafe) to map.find(err)
    added client::Schemas::get a single request handlers for the same cid/name
    added Ranger Column/s::get_{columns|ranges}(vec&) - for reporting command
    fixed Ranger Compaction at swc.rgr.compaction.range.max=1 (without delay)
    fixed (Experimental) Core::Mem::PageArena reset ptr at ItemPtr::release()
    fixed numericals Condition::sbs case of against Zero in modulus & fmod
    added unretriable case at ENOENT in FS::Interface::length
    added char[] Ranger::Range::CELLSTORES_RCVD_DIR
    added CELLSTORES_BAK_DIR check for init-create in Ranger::Range::load
    added Ranger Recover CellStore at CellStore::Readers::load_from_path
    changed Manager MngdColumns::m_expected_load to std::forward_list<cid_t>
    changed programs AppContext to wait for metrics finish before IO ctx stop
    changed Comm::client::ConnQueues stop out-of lock and host->stop at remove
    changed Manager Column::create mkdirs inclusive of RANGE_DIR
    changed Manager::Rangers endpoints eq-cond. to Comm::equal_endpoints(..)
    fixed Manager::Rangers missing cases of rgr->stop() at erase
    added ctor Manager::Ranger(const Ranger& other, const Comm::EndPoints&)
    removed Manager Ranger::set(Comm::EndPoints) - ctor Ranger with init_queue
    added Manager MngdColumns quit cases at no-run & cancel Load Expectations
    fixed Comm::client::Serialized dtor/stop at time of making new connection
    changed Manager Columns::get_need_health_check start by m_health_last_cid
    added Manager Columns::AssignGroup cls manages next ranges for assignment
    added source-files swcdb/manager/db/{Column,Range}.cc
    added Manager::Columns::assign_{add,remove}( Range::Ptr )
    changed Manager Columns::get_range(const rid_t rid) without bool initialize
    added Manager AssignGroup Columns::m_need_assign[4] [Master,Meta,Sys,Data]
    fixed Ranger Range::Block::add_logged(Cell&) a case at block got split
    added Ranger CompactRange use count barrier at finalize
    added Ranger::Blocks::release processing-state and skip at Compact Applying
    changed usage of DB::Specs::Column without Ptr shared_ptr<Column>
    added typedef Core::Vector<{c/rids_t,c/rids_t}
    added typedef Query::Update::Handlers::Base::Colms & DB::SchemasVec
    changed to STL Serial::Value Field_LIST_{INT64,BYTES} convert_to and ctor
    changed DB::Cell::Key::{equal,add,read,convert_to} to STL
    added Config::Property::from_string(const char*, T* value)
    added specialized extended const-expressions in Core::Vector<>
    added Core::Vector(initializer_list&&) and emplace(it, Args)
    added Core::Vector operator==(other) and pop_back()
    changed not-required cases of std::vector<> to Core::Vector<> source-wide
    removed Core::BufferT::length_base/grow
    added timer-synchronization MutexAtomic PeriodicTimer::m_mutex
    added std::ostream& operator<<(ostream&, EndPoint&)
    changed source-wide destructors to alway noexcept unless not specified
    added CXX-Flags -Wtrampolines -Wtsan (with GCC)
    added compatibility for -Wshadow (local,compatible-local,global) with GCC
    updated for Thrift v0.15.0
    added Python swcdb.thrift.pool.PoolStopping exception, raise with stop
    added SWC-DB Ruby 'swcdb' package and availability with gem list -r swcdb

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.5.4...v0.5.5)
******




### [SWC-DB v0.5.4](https://github.com/kashirin-alex/swc-db/releases/tag/v0.5.4) (2021-08-09)

    added SWC_FS_* Log & Stats for FileSystem::default_{write,combi_p/read}
    added STL Core::Vector<T, SizeT, GROW_SZ>
    added STL Core::VectorsVector<VectorsT, VectorT, SIZE, GROW, SPLIT>
    added 'Experimental'(not-used) classes Core::{Arrray/sArray/sVector}
    added support of operators {new,delete}/[] for align_val_t nothrow_t&
    changed private/inner/non-API std::vector<> types to Core::Vector<T>
    added possible std::vector optimizations (reserve/emplace_back/list-init)
    removed Common::Params::{HostEndPoints,ColumnsInterval,ColumnId,ColRangeId}
    added Config::Property::Value::V_GUINT16
    changed cfg swc.mngr.ranges.assign.Rgr.remove.failures to G_INT16
    changed ThriftBroker app-ctx std::vector<AppHandler::Ptr> to std::set
    changed DB::Cells::Mutable 'buckets' to Container _container
    added DB::Cells::Mutable::IteratorT get<T>{(Interval),(Key, ..offset)}
    removed size_t DB::Cells::Mutable::_narrow(..)
    added applicable constexpr source-wide excl. related on new/delete/memcpy
    added Core::VectorsVector<> test(test_big_vector) and basic std::map test
    fixed ZLIB Core::Encoder leak - Reset changed to End stream
    added Env::Bkr::is_accepting() to Handlers-request valid() state
    added G_INT32 configuration-property 'swc.mngr.column.health.checks.delay'
    added Range load "revision" state in Manager/Ranger/client-RangeLocators
    added int64_t Protocol::Mngr::Params::RgrGetRsp::revision
    added int64_t Protocol::Rgr::Params::RangeLoaded::revision
    added int64_t Protocol::Rgr::Params::RangeLocateReq::revision
    added flag Protocol::Rgr::Params::RangeLocateReq::HAVE_REVISION
    changed Protocol::Mngr::Handler::rgr_get rsp. with full range_{begin,end}
    added G_INT32 configuration-property 'swc.client.Mngr.range.master.expiry'
    added class client::Managers::MasterRangesCache
    added MasterRangesCache client::Clients::master_ranges_cache
    added Clients::mngr_cache_{get_(read,write},set,remove}_master(....)
    changed client::CachedRangers to Map non-pointer entry value
    renamed Mngr::Params::RgrMngId::Flag::'RS_' enums to 'RGR_'
    added Manager preload Columns SYS_STATS & SYS_DEFINE_LEXIC
    fixed Manager RangersResources timestamp by milliseconds instead nanos
    added Env::Rgr::m_in_process_ranges,in_process_ranges(,size_t)
    changed Env::Rgr::in_process(..) in Ranger::Range to 'in_process_ranges'
    changed algorithms process Conditions SBS and SPS
    fixed Cell::Serial::Value::Field_LIST_{INT64,BYTES} GT/GE require all True
    added Core::Vector<Field*> Serial::Value::Fields::_fields_ptr
    added Core::Vector<bool> Serial::Value::Field_LIST_{INT64,BYTES}::_found
    added allocate _fields_ptr at Fields::Fields(ptr, len)
    added allocate _found for SPS/SBS at Field_LIST_{INT64,BYTES}(bufp, remp)
    added Specs/Cell SERIAL value support for empty LIST_{INT64,BYTES}
    added DB::SchemaPrimitives + derived by DB::Schema & storage/serial order
    changed Ranger work with DB::SchemaPrimitives (RangeLoad & ColumnUpdate)
    removed Protocol::Rgr::Params::RangeLoad::cid
    added DB::Schema::Tags tags
    added DB::Schemas:: struct SelectorPatterns (consist names and tags)
    added DB::Schemas::matching support for SelectorPatterns::tags
    added Tags Comp[Comp'expr',...] SQL syntax support in Column-Selector
    added schema's column Tags support in Thrift Service
    added timeout-ms ARG for client::Clients::get_schema(..)
    added Core::QueueSafe<ItemT>::{push,push_and_is_1st}(ItemT&& item)
    removed bool Core::QueueSafeStated::deactivating(ItemT* item)
    changed Ranger Fragment::m_queue to ::queue<LoadCallback*>
    changed Ranger Fragment::load(..) to ARG (LoadCallback* cb);
    changed Ranger CommitLog::Splitter extends Fragment::LoadCallback
    changed Ranger CommitLog::Compact::Group extends Fragment::LoadCallback
    changed Ranger Range::BlockLoader extends Fragment::LoadCallback
    fixed Ranger _run_scan_queue with possible parallel execution
    added request_again on MNGR_NOT_INITIALIZED in req. Mngr Column{List,Get}
    fixed Manager expected columns to load completion (req. with uint64 total)
    changed Manager MngdColumns::m_expected_ready to m_expected_remain uint64
    removed Manager MngdColumns::m_mutex_expect and synced with m_mutex_active
    changed SWCDB-COPYRIGHT notice with github url Tag version LICENSE file
    added unique-fid for aligned_{min,max} field in Meta-Range serial-value
    fixed columns access on index in Manager and Ranger at many(+100K) columns
    changed proto Mngr::Req::ColumnList<> to buffered 1000-columns completion
    fixed Ranger compaction completion - decr. on-found & stop owns run-state
    changed lambda handlers in Comm::ConnHandler to added sub-classes:
        ConnHandler:: Sender_{noAck,Ack},Receiver_{HeaderPrefix,Header,Buffer}
    added overloaders ConnHandler::do_async_write(Sender_***)
    added overloaders ConnHandler::do_async_read(Receiver_***)
    changed ConnHandler::run_pending(ev) before next read()
    changed FsBroker & Broker app-ctx handlers to CommandHandler::operator()()
    added Ranger & Manager dedicated-handlers for commands posted on IO
    changed lambda accept to Acceptor::{Plain,Mixed} + operator()(ec,sock)
    changed source-wide timer lambda handlers to TimerTask::operator()(...)
    changed source-wide io-post lambda handlers to "Handler"::operator()(...)
    added System::Mem::_check_malloc(ts) 'used_reg' as by mallinfo2 uordblks
    added System::Mem::used_releasable & {more,less,adj}_mem_releasable(sz)
    added System::Mem::used_future & {more,less}_mem_future(sz)
    changed System::Mem::{avail,need}_ram used_reg with 'used_future'
    changed System::Mem::need_ram() return at most used_releasable
    removed System::Mem::{more,less,adj}_mem_usage(sz)
    fixed System::Mem::_release() 'need_ram' with 0-bytes presion
    fixed Ranger System memory track & Release procedure (only by releasable)
    added Ranger mem-release levels 0:cs 1:frags 2:blks 3:commit 4:structure
    added Python module swcdb.thrift.pool and classes Pool PoolService

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.5.3...v0.5.4)
******




### [SWC-DB v0.5.3](https://github.com/kashirin-alex/swc-db/releases/tag/v0.5.3) (2021-06-19)

    removed cases of Env::Config and Env::IoCtx in namespace Comm and client
    added Comm::IoContextPtr io_ctx storage in client::Clients class
    removed IoContextPtr client::Clients::default_io()
    added separate stop for io and services in client::Clients class
    removed cases of SWC_ASSERT in Env::FsInterface
    changed swcdb_cluser apply start port-arg by service-port cfg-property
    removed Comm::client::ConnQueueReqBase::{is_timeout,is_rsp}(ev)
    changed bool client::ConnQueueReqBase::insistent to virtual insistent()
    added Comm::AppContext handle_{established,disconnect} = 0 and overriders
    changed ConnHadler call on app_ctx->handle_established at new_connection
    changed ConnHadler call on app_ctx->handle_disconnect at do_close_run
    removed Comm::ConnHandler::run(const Event::Ptr& ev)
    removed Comm::ConnHandler::recved_{header,header_pre,buffer}
    removed enum Comm::Event::Type and Event rrror determined by ev->error
    added Core::Buffer(BufferT&&) and Core::BufferDyn(BufferDyn&&)
    added bool Update::Handlers::Base::Column::get_buff(... ,DynamicBuffer&)
    added encode-minimal Protocol::Rgr::Params::RangeQuerySelectReqRef
    added encode-minimal Protocol::Bkr::Params::CellsSelectReqRef
    added Params Rsp ctors with (int, const uint8_t*, size_t/,StaticBuffer&):
      Mngr::Params: RgrGet, Range{Unloaded,Create,Remove}, ColumnCompact
      Rgr::Params:  RangeQuery{Update,Select}, RangeLocate, ColumnCompact
      Bkr::Params:  Cells{Select,Update}Rsp
    changed Requests to a STL class with DataT requirements for request-type:
      Mngr::Req: RgrGet<DataT>,
                 Range{Unloaded, Create, Remove}<DataT>
                 Column{Get, List, Compact, Mng}<DataT>
      Rgr::Req:  Range{Locate, QuerySelect, QueryUpdate}<DataT>
      Bkr::Req:  Column{Get, List, Compact, Mng}<DataT>
                 Cells{Update, Select}<DataT>
    removed std::make_shared<..> use cases and changed ctors to Ptr(new ..)
    added bool client::Brokers::put(ReqBase::Ptr, Idx)
    added bool client::Manager::{put, put_role_schemas}(ReqBase::Ptr, ...)
    changed pref. some-defintions in header with SWC_CAN_INLINE of Performance
    added Column Existence check & Error handling in Committer & Scanner Query
    added Ranger option to issue Range Unload

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.5.2...v0.5.3)
******




### [SWC-DB v0.5.2](https://github.com/kashirin-alex/swc-db/releases/tag/v0.5.2) (2021-05-30)

    added SWC-DB Broker program (bin/swcdbBroker)
    changed default "swc.fs.broker.port" to port 14000
    added libswcdb (swc.cfg) configuration properties:
        * swc.bkr.{cfg,port}
    added libswcdb (swc.dyn.cfg) configuration properties:
        * swc.bkr.host
        * swc.client.Bkr.connection.{timeout,probes,keepalive}
        * swc.client.Bkr.comm.encoder
    added sbin/swcdb_cluster cfg "swc.cluster.bkr.host"
    added sbin/swcdb_cluster commands {start,stop,kill}-brokers
    changed Ranger run at most and at least one Compaction on low-memory-state
    changed Protocol::INST::Commands::max_command() to uint8_t MAX constexpr
    added Comm::Header::FLAG_RESPONSE_PARTIAL_BIT
    added DB::Cells::Mutable constructor with StaticBuffer
    added DB::Cells::Mutable::write_and_free(DynamicBuffer&, uint32_t)
    added new /db/client/service/{rgr,mngr,bkr} source paths
    added Clients::Flag to client::Query::{Update,Select}::Handlers::Base ctor
    added client::Clients ctors with additionally and only ContextBroker
    added helper client::Clients::*functions* to underlying types
    added Query::Update::Handlers::Base::{commit,_execute,default_executor}(.)
    added Query::Select::Handlers::Base::{scan,_execute,default_executor}(.)
    removed void client::Query::{Update::commit,Select::scan}
    added client/Query/Update/Handlers/Base.cc
    added Broker component to client::Query::Profiling
    changed print/display only with time consumed in client::Query::Profiling
    changed single handler for stats and definer columns in Metric::Reporting
    added client::SQL::Reader::read_uint64_t
    fixed missing/bad use cases of s/str-to-unsigned instead signed
    added "--with-broker" option to utilities programs and extended tests
    added BOOL cfg swc.{mngr,rgr,FsBroker}.metrics.report.broker=true
    added Base & Specialized Protocol::Mngr::Req::Column{Get,List,Mng,Compact}
    changed type of System::Resources::running to Core::CompletionCounter
    added Utils::shell::Interface support of "switch to CLI" command
    added client::Schemas::get(cid/name),set(schema/s)
    moved Comm::ConnQueueReqBase::valid() to Comm::DispatchHandler::valid()
    changed functions Comm::DispatchHandler::{handle,handle_no_conn} to pure
    removed source file core/comm/DispatchHandler.cc
    added cond. case of client::ConnQueue::ReqBase::valid() to Mngr::Req proto
    added FS::FileSystem::settings & FS::Interface ctor Config::Settings::Ptr
    updated for ASIO-0.18.2
    resolved issue #9 SWC-DB Query Broker - Program

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.5.1...v0.5.2)
******




### [SWC-DB v0.5.1](https://github.com/kashirin-alex/swc-db/releases/tag/v0.5.1) (2021-05-16)

    added source core/Comparators.cc
    added reasonable cases of std::string::reserve
    removed namespace FileUtils & added FS::FileSystemLocal specialized funcs
    added cmake definer -DSWC_WITHOUT_THRIFT=ON/OFF
    added host-arg Comm::AppContext::init(host, EndPoints&)
    added Comm::AppContext::{net_bytes_sent,received,accepted}
    added ConfigSSL::need_ssl(const EndPoint& local, const EndPoint& remote)
    fixed Client/Server need-ssl, local!=remove and remote not in secure nets
    added client:: Clients::stop() and bool Clients::stopping()
    added Error::CLIENT_STOPPING & cases to skip/stop clients requests
    added callback if Clients::stopping() before Req::MngrActive
    fixed client::Query::Select::scan completion at many col-intvals
    fixed client::Query::Select::Handlers:Common, return at not valid_state
    fixed misalignment/cast of cell-ttl seconds(uint32_t) to nanoseconds
    added Core::Time::Measure<,> and typedef Time::Measure_{ns,us,ms,sec}
    added virtual FS::Type FS::FileSystem::get_type_underlying()
    added BOOL Configurations Properties:
        swc.{rgr,mngr,fsbroker,ThriftBroker}.metrics.enabled
        swc.fs.{local,hadoop,hadoop_jvm,ceph,broker}.metrics.enabled
    added G_INT32 Configurations Properties:
        swc.{rgr,mngr,fsbroker,ThriftBroker}.metrics.report.interval
    added struct FS::Statistics,::Metric,::Tracker - fs/Statistics.{h,cc}
    added fs/Logger.h (Common FileSystem Logging Macros)
    added struct Comm::Protocol::{FsBroker,Mngr,Rgr}::Commands
    added {db,fs/Broker}/Protocol/Commands.cc
    added System Reserved Column now is 10 columns (CID[1-10])
    renamed files & namespace MetaColumn to SystemColumn
    added cid_t SystemColumn::SYS_CID_{STATS,DEFINE_LEXIC,END}
    changed SYS_CID_STATS to cid=9 and SYS_CID_DEFINE_LEXIC set to cid=10
    changed class Resources namepace to System from Common
    added class System::Resource::{CPU,Mem} and struct System::Notifier
    changed System::Resources stoage with Resource::{CPU cpu, Mem mem}
    added Common::Resources& Env::{Mngr,FsBroker}::res()
    added namespace client::Query::Update::Handlers::Metric (Metrics.h,cc)
    added include/common/sys/MetricsReporting.h
    added class {Ranger,Manager,FsBroker,Thriftbroker}::Metric::Reporting
    added class SWC::Env::ThriftBroker - thrift/broker/ThriftBrokerEnv.h
    added Statistics Client CLI "SWC-DB(statistics)>" (bin/swcdb --statistics)
    resolved issue #6 Services & Components statistic
    changed Config::Properties::get_*,defaulted,to_string to const scope
    changed use cases of Env::Clients in libswcdb.so to client::Clients::Ptr
    moved Query/*{Update,Select}*Handlers.* to dirs {Update,Select}/Handlers/*
    renamed Query/Update/Update.{h,cc} to Query/Update/Committer.{h,cc}
    renamed Query/Select/Select.{h,cc} to Query/Select/Scanner.{h,cc}

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.5.0...v0.5.1)
******




### [SWC-DB v0.5.0](https://github.com/kashirin-alex/swc-db/releases/tag/v0.5.0) (2021-04-26)

    changed Comm::ConnHandler use _buff_header[MAX_LENGTH] instead temp allocs
    fixed use of invalid Iterator in Mutable::write_and_free
    added move ctor/operator Specs::{Interval,KeyIntervals,Key,Fraction,Values}
    changed DB::Specs::KeyIntervals Vector to non-Ptr Specs::KeyInterval
    added class KeyGenerator to swcdb_load_generator
    changed workload by Distribution and Course in swcdb_load_generator
    added Ranger call CompactRange::quit() at Compaction::stop()
    added Ranger {Compaction,Env::Rgr}::log_compact_{possible,finished}()
    added Ranger configuration property 'swc.rgr.compaction.commitlog.max'
    added compact-possible cond. in Ranger::CommitLog::Fragments::try_compact
    changed FS::Dirent structure - skip dir length encoding & time_t to int64
    added direct POSIX ::readdir handling in FS::FileSystemLocal::readdir
    removed FileUtils::readdir & fixed bad-vector of POSIX dirent
    changed API of SWC::DB::client::Query::Select :
      added Handlers namespace to SWC::DB::client::Query::Select
      added Handler classes Base{UnorderedMap,SingleColumn},Common
      changed Query::Select::Scanner class ctor takes Handlers::Base::Ptr
      added Query::Select::scan(..) default impl./overloaders of executions
    changed bool Specs::Key::is_matching(KeySeq,Cell::Key&) to const scope
    removed Cell::Key 'range_offset' specification from DB::Specs::Interval
    added DB::Mutable::_narrow(const Specs::Interval&) improved narrow options
    added atomic uint64_t 'cache' to SWC::client::Query::Profiling::Component
    added profile aggregation with operator+= to SWC::client::Query::Profiling
    added cached rsp-type profiling support for DB::client::Query
    added specialized select-handler class Ranger::Query::Select::CheckMeta
    fixed single-Ranger-runtime shutdown-seq - halt Meta-Check if shuttingdown
    fixed Query::Select::Scanner error-handling over a retry on req-base(fb)
    changed API of SWC::DB::client::Query::Update :
      added Handlers namespace to SWC::DB::client::Query::Update
      added Handler class Base{ColumnMutable,UnorderedMap,SingleColumn},Common
      changed Query::Update::Updater cls to Committer takes Handlers::Base::Ptr
      added Query::Update::commit(..) default impl./overloaders of executions
    added Ranger bool Range::state_unloading() const
    added Ranger handler specializations of client update query:
      added folder ranger/queries/update
      added classes/files Ranger::Query::Update::{BaseMeta,CommonMeta}
      added Ranger::Range sub-classes MetaRegOn{LoadReq,AddReq}
    changed possible cases of std::function<..> to pass by move
    changed FS::FileSystem pass FS::Callback::*Cb_t by move
    fixed Ranger case of CellStore Read with many index-blocks (was only last)
    added std::string format_unsafe(const char *fmt, ...)
    added -Wformat-{signedness,nonliteral} compiler Flags & fixed bad cases
    changed Ranger CommitLog::Fragment Constructor move DB::Cells::Interval
    changed Ranger pass CellStore::Block::Header with move
    added FS::SmartFd constructor for a moveable filepath
    removed Config::Properties::to_string_all
    changed use const char* as input where reasonable in Config:: namespace
    added core/Comparators_basic.h with supporting {str,mem}_{cmp,eq}
    changed ::memcmp,str{n,ncase}cmp via specialized functions in Condition::
    added FsBroker shuttingdown and processing state
    added bin/swcdb/ targets with -DSWC_ENABLE_SANITIZER definer
    fixed full-shutdown sequences at Sanitizer Enabled (without quick_exit)
    changed some simple functions moved to header files
    changed Ranger release previous Blocks at/after Blocks::ScanState::QUEUED
    added QueueSafeStated<T>::{push,activating,deactivating} for moveable item
    changed ConnHandler::Pending to nonHeap separate struct Outgoing & Pending
    fixed Ranger CommitLog::Fragment::m_processing synchronization
    added core/MutexLock.h (shared,unique,scoped lock scopes with noexcept)
    changed std::{shared,unique,scoped}_lock to corresponding Core::*Lock
    added Ranger Fragments::_remove(int&, Fragments::Vec&, Core::Semaphore*)

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.4.19...v0.5.0)
******




### [SWC-DB v0.4.19](https://github.com/kashirin-alex/swc-db/releases/tag/v0.4.19) (2021-03-23)

    added Comm::Protocol::Rgr::Params::RangeUnoad.h
    changed RANGE_UNLOAD command's req/rsp with RangeUnoad params
    added DB::Types::MngrRange::State::MERGE
    added class Comm::Protocol::Rgr::Params::RangeIsLoadedRsp
    changed RANGE_IS_LOADED command's rsp with RangeIsLoadedRsp params
    added Manager Request Comm::Protocol::Rgr::Req::RangeUnoadForMerge
    moved Ranger::Range::{CELLSTORES_DIR,LOG_DIR,RANGE_FILE} to DB::RangeBase
    added Ranger CompactRange cond. split immediate if size_bytes > cfg-cs-size
    added Manager ColumnHealthCheck sub-classes ColumnMerger & RangesMerger
    added void Manager::Rangers::wait_health_check(cid_t cid)
    added Ranger Range at update-schema cond. compact if TTL changed from None
    added bool Ranger::CommitLog::Fragments::empty()
    added Ranger::Blocks::reset_blocks()
    removed 0-bytes case handling from Ranger::Blocks::release(size_t bytes)
    added quit-time option for Ranger::Blocks::wait_processing(int64_t ns=0)
    fixed FS IO handlers exhaustion in Ranger::CommitLog::Fragment::write
    fixed libswcdb_core queue test, front access failure at one-thread run
    resolved issue #3 Merge of empty Range
    added Env::Rgr::scan_reserved_bytes()/_sub/_add(uint32t bytes)
    added void Env::**-majority-of-classes**::reset()
    fixed mem-leak in class DB::Cells::Mutable dtor, an init. bucket was left
    fixed mem-leak in Comm::Resolver, cases of missing freeaddrinfo(result)
    fixed mismatched delete in DB::Cell::Key::remove for new[] array
    fixed misaligned i8 pointer in Core::fletcher32 for i16 cast usage
    fixed overflow on uint16_t x uint16_t in Manager::RangersResources
    changed to capital letter case FS::{ID_SPLIT_LEN,ID_SPLIT_LAST}
    changed FS::Interface::set_structured_id structure sizing by iterator
    added Config::Property::Value::assure_match(Type t1, Type t2)
    added macro SWC_ASSERTF(_e_, _fmt_, ...) to core/Exception.h
    fixed mismatched swc.ThriftBroker.timeout cfg type in init of ThriftBroker
    fixed ctor Cell::Cell(const uint8_t**, size_t*, bool) bad 'own' ARG usage
    added build-wide Cmake definer -DSWC_ENABLE_SANITIZER=address/thread
    removed Clang support for insistent-malloc at SWC_MALLOC_NOT_INSISTENT=OFF
    fixed missing mutex over actions on timer in class Comm::ConnQueue
    changed Ranger::Blocks 'need_ram' for preload by Rgr::scan_reserved_bytes
    added flag FS::OpenFlags::WRITE_VALIDATE_LENGTH
    added LENGTH check to FsBroker write handler & FileSystem::default_write
    added WRITE_VALIDATE_LENGTH Fd flag to Ranger Fragment::make_write/write
    added option WITH_MALLOC to GET_TARGET_LINKS macro in cmake/Utils.cmake
    removed unused Flags from Comm::Header & removed Flags enum
    added Error::SERVER_MEMORY_LOW
    changed Manager evaluate total-count of mngr-hosts excluding at OFF-state
    added Error::SERVER_MEMORY_LOW cond. in Ranger RANGE_QUERY_SELECT handler
    removed StaticBuffer 'input' from Ranger::Callback::RangeQueryUpdate
    added int32_t FS::SmartFd::invalidate() & pre-invalidations at closing Fd
    fixed a missing case of sql_list_columns on patterns in ThriftBroker
    added Column-Selector with pattern syntax to sql_compact_columns
    added static_asset on Arch not Little-Endian/64+ bits/128 bits long double
    changed Comm::client::ConnQueue evaluate for closure after keep-alive reach
    changed Comm::ConnHandler always awaits for a read event
    removed Comm::ConnHandler::{accept_requests(),close(),m_accepting,m_read}
    changed Comm::ConnHandler::disconnected() to callable multiple-times
    fixed unsynchronized state of m_check_log in Ranger::BlockLoader::load_log

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.4.18...v0.4.19)
******




### [SWC-DB v0.4.18](https://github.com/kashirin-alex/swc-db/releases/tag/v0.4.18) (2021-03-07)

    added swc-db/.github/workflows/ci.yml, to activate requires [TEST COMMIT]
    added packaging/debian
    changed Config::Settings reduced alloc with move to emplace move-ctor
    added core/comm/asio_wrap.h isolation of ASIO include
    added environ SWCDB_CLUSTER_SSH_CONFIG to sbin/swcdb_cluster
    changed fixed cfg-values/paths to dynamic-input sbin/swcdb_cluster
    added validate transport to socket cast in ThriftBroker
    changed condition -LT/-GT to -NE at comparison with end-iterator
    added ctor client::Query::Update::Locator::Locator(without a key_finish)
    adjusted further the standard to comply with Warning Flags clang-additio
    renamed response() to complete() of Ranger::Callback::ManageBase derived
    changed order of #include<*stream> & removed unused/redundant #include<..>
    added core/Malloc.cc
    added fast-alloc route in DynamicBuffer at no BufferT::base
    changed some encoded_length() to uint32_t, avoid overflow on sum of size_t
    fixed build-linkage at -DSWC_IMPL_SOURCE=ON
    added Comm::EndPoints Serialization::en/decode
    changed ctor Comm::Buffers at ReqBase constructor
    removed avoidable SWC_ASSERT at run-time
    added system_clock capability check at build-time
    updated Java org.swcdb.thrift for Thrift 0.14.0 version
    added pom_sonatype.xml & SWC-DB Thrift Client to Maven Central Repository

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.4.17...v0.4.18)
******




### [SWC-DB v0.4.17](https://github.com/kashirin-alex/swc-db/releases/tag/v0.4.17) (2021-02-14)

    moved specific functions to header impl. in DB::Cells::Mutable and DB::Cell
    added void Core::BufferDyn::take_ownership(BufferT& other)
    added classes Core::BufferStream{Out,In} + _ZSTD
    added 'zst' ext. support for TSV files in dump & load commands of SWC-DB(client)>
    added FORMAT syntax: split=Bytes ext=zst level=INT in 'dump' command
    added support in dump & load for using specific file-system for Write/Read
    added FS syntax: fs=Type in 'load' & 'dump' command
    changed PATH syntax to 'path=/PATH' in 'load' & 'dump' command
    changed FS::Interface::Ptr to std::shared_ptr<FS::Interface>
    fixed SONAME in cmake INSTALL_LIBS macro, bad use case of CMAKE_MODULE_PATH
    added OUTPUT_FLAGS to shell::DbClient 'list columns' command
    removed SWC_LOGF from checksum_i32_chk + added checksum_i32_log_chk (perf.)
    updated for Apache-Thrift 0.14.0 + github.com/apache/thrift/pull/2318
    added Move Contructors and Assignments in Thrift::Service

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.4.16...v0.4.17)
******




### [SWC-DB v0.4.16](https://github.com/kashirin-alex/swc-db/releases/tag/v0.4.16) (2021-02-05)

    added CXX compiler Warning flags:
        -Wpedantic -Wnon-virtual-dtor -Wcast-align
    added CXX compiler Warning flags with relaxed case of -Wno-error=*:
        -Wnoexcept -Wsuggest-override -Wuseless-cast -Wold-style-cast
        -Wnull-dereference -Wstrict-null-sentinel -Wzero-as-null-pointer-constant
        -Wduplicated-cond -Wduplicated-branches -Wlogical-op
    changed to comply with added warning flags by adjustments and fixes
    added T* Config::Property::Value::get_pointer<Property::Type>(Value::Ptr)
    added DB::Cell::get_value(std::string& v)
    added move ctor and operator for DB::Cells::Cell and DB::Cell::Key
    fixed libpam_swcdb_max_retries with latest SWC-DB Thrift Service structure
    changed Comm::Buffers::buf_header to bytes-array (uint8_t[Header::MAX_LENGTH])
    fixed possible 0-length in Comm::ConnHandler::recved_header_pre
    removed Comm::Serializable base from FS::Direct and DB::Specs{Scan, Column}
    changed 'len' arg in DB::Cell::Key::{insert,add} to uint24_t
    changed Core::MutexSptd::lock() noexcept & added MutexSptd::lock_except()
    added class Core::MutexSptd::scope_except
    changed mutex to MutexSptd in Properties, Thrift::AppHandler & Mngr::Groups
    changed cmake, set USE_GNU_READLINE if EDITLINE not-found
    removed 'source ~/.bashrc;' from `start-fsbrokers` in sbin/swcdb_cluster
    added FS::FileSystem::default_{write,read,pread,combi_pread}
    added impl. in Ceph/Hadoop/Local calling on FS::FileSystem::default_*
    added DB::Cell::Key::add(std::vector<KeyVec::Fraction>&)
    added DB::Cell::Serial::Value::Field_KEY::decode(uint8_t**, size_t*, bool)
    added Serial::Value::{read_field_id,skip_type_and_id}(const uint8_t**,size_t*)
    added DB::KeySeq::compare(Key&, Key&, Condition::Comp break_if, ..)
    removed unused max_revs, type, reset() & configure() from DB::Cells::Result
    added void DB::Cell::Key::copy(uint24_t after_idx, const Key&)
    added default Comparator to Specs::Values::add(Condition::Comp=Condition::EQ)
    changed SYS-Columns cids[1-8] to Column::SERIAL, query on value possible
    added Ranger::Range fix duplicate rid MetaData cells (delete & reg. range)
    added cmake option -DSWC_BUILD_PKG=PKGNAME
    added cmake & build Definers -DSWC_PATH_{ETC,LOG,RUN}=".."
    added folder packaging/archlinux/ with the corresponding PKGBUILD folders

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.4.15...v0.4.16)
******




### [SWC-DB v0.4.15](https://github.com/kashirin-alex/swc-db/releases/tag/v0.4.15) (2021-01-20)

    updated for TCMalloc 2.8.1 version
    updated for ASIO 1.18.1 version
    removed BitFieldInt<T, SZ> destructor
    added Serialization::fixed_vi_i{24,32,64} consumes length-byte + actual
    added Serial Field LIST_BYTES in DB::(Specs,Cell}::Serial::Value
    added constructor DB::Specs::Interval(Types::Column col_type)
    added support for multiple DB::Specs::Value & added class Specs::Values
    added specialized Specs::Value::is_matching_{plain, serial, counter}
    added struct DB::Specs::Value::TypeMatcher (a defined base instead void*)
    added domain Condition Comparators SBS, SPS, POSBS, POSPS, FOSBS, FOSPS
    added bool client::SQL::Reader::is_numeric_comparator(Condition::Comp&)
    added NE(!=) condition support in DB::Specs::Value of SERIAL Column Type
    added support to SWC-DB Thrift Service for Serial Cell-Value
    added Encoder support in Thrift Service for update cell(UCell,UCellSerial)
    removed 'optional' Thrift annotation from Container,Struct,Binary,String
    added generated SWC-DB Thrift Service for Rust

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.4.14...v0.4.15)
******




### [SWC-DB v0.4.14](https://github.com/kashirin-alex/swc-db/releases/tag/v0.4.14) (2021-01-13)

    added class Core::StateRunning & changed running-state vars to StateRunning
    added core/Atomic.h & changed use cases of std::atomic<> to Core::Atomic<>
    changed possible returns of std::string to const char* as noexcept
    changed Ranger::CellStore::Read::m_queue to QueueSafeStated<Block::Read::Ptr>
    changed Ranger::Blocks::wait_processing() to shorter check-up and dual
    changed Ranger{Block,Fragment,Cs::Block} m_err,m_state,m_processing to atomic
    added class Core::QueuePointer<PtrT>
    fixed Ranger::CompactRange::add_cs, after finalize add to vector<Write::Ptr>
    fixed Ranger::CompactRange::initialize, bad usage of signed-size 'split_at'
    changed Ranger::CompactRange::InBlock to QueuePointer<InBlock*>::Pointer base
    changed Ranger::CompactRange::m_q_{intval,encode,write} to QueuePointer<>
    merged Ranger class RangeQueryUpdate & ReqAdd with base QueuePointer<>::Pointer
    changed Ranger::Range::m_q_add to QueuePointer<Callback::RangeQueryUpdate*>
    added Ranger g_i8 configuraion-property swc.rgr.Range.req.update.concurrency
    added Ranger::Range::m_mutex_intval_alignment & adjusted use of m_mutex_intval
    changed Ranger::Range not-mutex based functions to an internal '_'-prefixed
    added temp align_{min,max} in Ranger::Range::run_add_queue
    changed Ranger cases of read-cells loops with a wider try-block over a loop
    changed possible DB::Cells:Cell Conctructor cases to use the buffer-ctor
    added void DB::Cell::Key::_free(), Core::Buffer<T>_free() and Cell::_free()
    added bool DB::Interval::align(KeyVec& min, KeyVec& max)
    changed cases of str-compare to case-insensitive
    changed strerror to Error::get_text
    added option to create a schema with designated cid in Manager::MngdColumns
    added condition ev->expired() in Programs-AppContext - skip run handler
    added use-count of hdfsFile in FileSystemHadoopJVM::SmartFdHadoopJVM
    fixed FS::FileSystemHadoopJVM cases of invalidated hdfsFile instance usage
    fixed FsBroker handlers using 'close', remove fd from map after close
    fixed client::Query::Select interval-offset clear key at range-key_end is GE
    changed client::Query::Update commit_or_/wait_ahead_buffers accept `from` count
    moved KeySeq::is_matching(Specs::Key& key, Cell::Key& key) to Specs::Key
    added void* Fraction::compiled precompiled case of Condition::RE
    changed Specs::KeyIntervals::is_matching switch over keys loop
    changed Buffer<T,SZ>::size to 56 Bit-Fields (a 16-byte struct)
    added Buffer<T,SZ>::assign(const value_type* data, size_t len)
    added sbin/swcdb_cluster tasks with 'kill' a not gracefull stop with SIGTERM
    changed Condition::is_matching(comp, int64, int64) to template<T>
    changed 'double' serialization in Core::Serialization
    added Cell-Value encoding option in Cell serialization
        added sql-update sytax with ENCODER (after value)
        added TSV field 'Encoder' - '1' for encoded
        added client::SQL token OUTPUT_NO_ENCODE
        added control DB::Cells::HAVE_ENCODER & HAVE_ENCODER_MASK
        added DB::Cells::OutputFlag::NO_ENCODE
        added Cell::set_value(Types::Encoder enc, const uint8_t*, uint32_t);
        added Cell::set_value(Types::Encoder enc, const std::string&)
        added Cell::get_value(StaticBuffer& v, bool owner=false)
        added bool Cell::have_encoder() const;
        added load_generator option 'gen-cell-encoding'
    added DB::Types::Column::SERIAL
        added {Cell,Specs}ValueSerialFields.{h,cc}
        added support for Serial types: INT64 ,DOUBLE ,BYTES ,KEY, LIST_INT64
        added sql-{Select,Update} sytax for SERIAL value expressions
    added bool Specs::Value::is_matching(const Cells::Cell& cell)
    added void* Specs::Value::compiled precompiled specs cond. RE,COUNTER,SERIAL

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.4.13...v0.4.14)
******




### [SWC-DB v0.4.13](https://github.com/kashirin-alex/swc-db/releases/tag/v0.4.13) (2020-12-18)

    changed Zero/NotZero conditions to boolean evaluation
    added Manager MngrRole::are_all_active(Mngr::Groups::Vec&)
    moved namespace & file Stats::CompletionCounter to Core::CompletionCounter
    changed Manager class Rangers mutexes to SWC::Core::MutexSptd
    renamed Types::MetaColumn::get_meta_cid to get_meta_cid_str
    added cid_t Types::MetaColumn::get_meta_cid(KeySeq col_seq)
    changed Query::Select into Single Scanner better failure-tolerance handling
        fixes Query::Select scan at offset over shutdowns
    added support for DB::Specs::Interval to work with several keys-intervals
        added file & calss DB::Specs::KeyIntervals
        changed DB::Specs::Interval key_{start,finish} to KeyIntervals
        extended client::SQL::QuerySelect parser
        added struct SpecKeyInterval to Thrift Service
        changed Thrift SpecInterval key_{start,finish} to SpecKeyInterval
    added input Mitigation in Ranger CompactRange at log above CS(sz X ~max)
    removed Ranger::Block::State::REMOVED
    added bool DB::Types::is_fc(KeySeq typ)
    added uint8_t options to DB::Specs::Interval storage used as bitwise
        added SQL select, range-interval with token-option 'range_end_rest'
        added flag Protocol::Rgr::Params::RangeLocateReq::RANGE_END_REST
        added DB::Specs::Interval::{has,set}_opt__{key_equal,range_end_rest}
    extended Range Locator & Range Scan determination of reached-end & stop
        added bool& stop at DB::Specs::Interval::is_matching
        added functions DB::Interval::apply_possible_range*(..)
        added spec.apply_possible_range_pure at RangeQuerySelect in Ranger
        added DB::Interval::is_{in,matching}* specialization
    changed empty-types to minimal required log-info in DB::Specs classes
    fixed Ranger Range processing state & processing at block preload
    added operators new[], delete, delete[] to swcdb/core/Malloc.h
    added build support with MIMALLOC
    changed Ranger Fragment::load to Async FS calls with fs()->combi_pread
    changed Ranger CellStore Block Read::load to Async FS calls
    added Ranger Fragment::remove(int&, Core::Semaphore* sem)
    added FS::Interface::remove(RemoveCb_t&, string&)
    added FS::Interface::close(CloseCb_t&, SmartFd::Ptr&)
    added function FS::FileSystem::combi_pread (open+pread+close)
    added FsBroker protocol command FUNCTION_COMBI_PREAD
    changed Ranger Range add/scan request at compact without waiting thread
    changed Ranger CommitLog::Compact to atomic completion & workload
    changed Comm::IoContext Executor type by asio::thread_pool
    changed Ranger Range Block marked LOADED at Block::load_final by CommitLog
    changed Ranger CommitLog::Fragment::Ptr to std::shared_ptr<Fragment>
    added Ranger skip Block preload at commitlog compacting
    added Ranger CommitLog::Fragment marked-remove state
    changed Ranger Block class stores BlockLoader* m_loader with q_req
    added client::Query::Select::dispatcher_io a Comm::IoContextPtr
    changed Core::Completion to a atomic lockfree
    changed client::Query::Update errors handling with switch
    added multi-columns work-load support to swcdb_load_generator
    added Ranger CommitLog configurable properties:
        g_i8 swc.rgr.Range.CommitLog.Compact.cointervaling
        g_i8 swc.rgr.Range.CommitLog.Fragment.preload
    added log_compact_cointervaling & log_fragment_preload to:
        DB::Schema, Thrift::Service::Schema
    changed CommitLog::Compact & BlockLoader to work with Ranger cfg values
    added SQL Schema Syntax support for log_compact & log_preload

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.4.12...v0.4.13)
******




### [SWC-DB v0.4.12](https://github.com/kashirin-alex/swc-db/releases/tag/v0.4.12) (2020-11-13)

    added Manager feature to rebalance Rangers' ranges load
    added config property swc.mngr.rangers.range.rebalance.max
    added Ranger check and fix range MetaData on Range-load (select and update)
    added Manager recover Master&Meta columns (full-recovery at Ranger Range-Load)
    added Protocol::Mngr::Params::RgrMngId::Flag::MNGR_ACK
    fixed Protocol::Mngr::Command::RGR_MNG_ID request, params and handler
    changed enum MngrRanger::State to uint8_t in NS DB::Types::MngrRangerState
    added state DB::Types::MngrRangerState::SHUTTINGDOWN
    added Manager handling cases of a Ranger at MngrRangerState::SHUTTINGDOWN
    fixed Manager ColumnHealthCheck completion state
    fixed Manager RangerState MARKED_OFFLINE bit value
    added Manager load 1st SysColm Columns::get_next_unassigned(&waiting_meta)
    changed Comm::Buffers calc. nchunks with div & mod instead double
    added asio::strand<> to ConnHandlerSSL
    changed Comm::ConfigSSL asio::ssl::context usage/init to in-class storage
    added Resolver::get_local_networks
    added no need ConnHandlerSSL when client & server on the same add
    deprecated cc-files for FsBroker Requests classes
    added FsBroker A+Sync specialized Requests and common handlers to Req::Base
    added Comm::Buffers ctors with uint64_t cmd, uint32_t timeout (for Req.)
    added Comm::Buffers ctors with Event::Ptr (for Rsp.)
    added bool Comm::Buffers::expired(), fixing to-not-send expired Events
    changed make Comm::Buffers with specialization by Req/Rsp ctor
    added separate Env::Mngr & Env::Rgr default IoContext inplace of Env::IoCtx
    added header base helper class Core::NotMovableSharedPtr
    changed {Column,Range}::cfg to ColumnCfg::Ptr (NotMovableSharedPtr)
    added Types::MetaColumn definitions cid_t CID_{MASTER,META}_{BEGIN,END}
    added client::Query::Select::ScannerColumn::clear_next_calls()
    added new configuration properties:
        swc.rgr.clients.handlers
        swc.mngr.clients.handlers
    added Ranger Callback::ManageBase and sequential Column Mamangement
    added Manager ColumnMng::Function::INTERNAL_EXPECT (Colms Load Completion)
    fixed Manager MngdColumns::initialize() pending-schemas loader
    fixed completion at client::Query::{Select & Update} by CompletionCounter
    added client::Query::Select::response_if_last()
    improved Managers Columns management(update-status&-ack in-ring by req_id)
    changed avoid unreasonable queueing on io-context in Ranger & Manager
    added DB::Key::add(std::vector<std::string>::const_iterator {cbegin,cend})
    fixed cyclic-rsp of Unknown Error in FileSystemHadoopJVM at hdfsCloseFile
    added read & load example - 'Criteo 1TB Click Logs dataset'
    changed Ranger Blocks::remove, without rm files (Range will rm the folder)
    added Manager at Column remove, Query MetaData & delete cells remained
    added Ranger at RangeSplit apply compacting state for the new-range
    added optionally to pass the Comm::IoContext::Ptr to use in client::Update
    changed Ranger::Range::on_change requires a Callback
    changed all Manager and Ranger client::Query::{Select/Update} with async cb
    fixed Manager get_schema hadler check first is_schema_mngr state
    fixed Comm::client::ConnQueue without put(req) back at operation_aborted
    changed CommitLog::Splitter to work by Semaphore work-load state
    added size_t pre_acquire option to Semaphore ctor
    added CounT CompletionCounter::{in,de}crement_and_count()
    fixed ThriftBroker process_results for CompactResults
    added configuration option swc.ThriftBroker.connections.max
    added default swc.ThriftBroker.cfg.dyn=swc_thriftbroker.dyn.cfg file
    added thrift::transport::TSocket to Thrift::AppHandler Broker ctor&storage
    added Thrift-Broker LOG_INFO Open & Close Connection + total connections
    added Thrift-Broker shuttingdown at server stopped-serving
    fixed Thrift-Broker shutdown, stop TThreadPoolServer before ThreadManager

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.4.11...v0.4.12)
******




### [SWC-DB v0.4.11](https://github.com/kashirin-alex/swc-db/releases/tag/v0.4.11) (2020-10-14)

    added Encoder support to Communications Protocol, old Proto not compatible
    added configuration properties:
      - swc.client.Mngr.comm.encoder
      - swc.client.Rgr.comm.encoder
      - swc.fs.broker.comm.encoder
      - swc.mngr.comm.encoder
      - swc.rgr.comm.encoder
      - swc.FsBroker.comm.encoder
    added Config::Property::V_GENUM::Ptr cfg_encoder to Comm::AppContext class
    changed client::Mngr::AppContext inherits client::ContextManager
    added classes client::ContextManager and client::ContextRanger
    added separate AppContext for Ranger and Manager in client::Clients
    changed Clients ctor from AppContext::Ptr to Context{Manager,Ranger}::Ptr
    added cfg_encoder init by role to all AppContext based on Comm::AppContext
    added struct Comm::BufferInfo file comm/HeaderBufferInfo.h/cc
    changed buffers details storage in class Comm::Header to BufferInfo type
    added Core::Encoder::Type ConnHandler::get_encoder()
    added call to cbuf->prepare in ConnHandler::write_or_queue
    added void Event::decode_buffers()
    added Build-Config definer SWC_DEFAULT_ENCODER=PLAIN|ZLIB|SNAPPY|ZSTD
    added Error::RANGE_BAD_CELLS_INPUT
    added uint32_t cells_added to Protocol::Rgr::Params::RangeQueryUpdateRsp
    added try block for cell.read in Ranger::Range::run_add_queue
    added args uint32_t skip and bool malformed to
        Mutable::add_raw(const DynamicBuffer&, const Key&, const Key&, ..)
        ColCells::add(const DynamicBuffer&, const Key&, const Key&, ..)
    added case for Error::RANGE_BAD_CELLS_INPUT in client::Query::Update
    fixed Resend Cells Count in load_generator and in SWC-DB(client) shell
    added Encoder::encode(.., bool no_plain_out=false) option
    fixed Ranger/s shutdown process sequence
    added configuration property swc.fs.hadoop_jvm.reconnect.delay.ms

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.4.10...v0.4.11)
******




### [SWC-DB v0.4.10](https://github.com/kashirin-alex/swc-db/releases/tag/v0.4.10) (2020-10-05)

    added file-pid option in Settings::init_process
    extended sbin/swcdb_cluster deploy command with Tar archive option
    changed unique_lock where possible and lock_guard to scoped_lock
    changed mutex to Core::MutexAtomic in common::Stats and for timers
    changed MutexAtomic to work with std::atomic_flag
    moved all swcdb_core_config target related to SWC::Config namespace
    moved all swcdb_core_comm target related to namespace SWC::Comm
    moved class RangerEnv to namespace SWC::Env under class-name Rgr
    renamed class Comm::CommBuf to Comm::Buffers
    renamed class Comm::CommHeader to Comm::Header
    moved FS::SmartFdHadoop/JVM to sub-class FileSystemHadoop/JVM
    extended SWC::Thrift::Service documentations
    moved namespace Types::Encoding to SWC::Core::Encoder
    moved enum Types::Fs to SWC::FS::Type
    moved namespace Types to SWC::DB::Types
    added namespace Common for include/swcdb/common
    adjusted ifndef & define from 'swc_*' to 'swcdb_*'
    moved Stat, FlowRate, CompletionCounter from core/ to common/Stats
    moved Serializable to namespace SWC::Comm
    moved namespace SWC::Protocol to SWC::Comm::Protocol
    moved namespace FsBroker::Protocol to Comm::Protocol::FsBroker
    moved swcdb_core target related to namespace SWC::Core
    added db/Types/Encoder.h (Encoder with the schematics of the DB::Types)
    changed default include of Error.h to Exception.h
    renamed namespace Logger to Core
    renamed Mutex to MutexSptd
    moved u/int24/40/48/56_t to SWC namespace
    renamed LockAtomicUnique to MutexAtomic
    removed ErrorCodes.h moved to Error.h
    moved Exception to file Exception.h under namespace Error
    changed enums base to uint8_t where applicable
    moved Range::type & Range::meta_cid to ColumnCfg class storage
    optimized Core::checksum32 & Condition::memcomp
    extended documentations and C++ source-code documentations
    renamed Env::Fds to Env::FsBroker and Env::FsBroker::get() to fds()
    added to DB/Types/* const char array type representations

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.4.9...v0.4.10)
******




### [SWC-DB v0.4.9](https://github.com/kashirin-alex/swc-db/releases/tag/v0.4.9) (2020-09-28)

    fixed parse_ns & fmt_ns with ns at neg and ms/us float point in SWC::Time
    added include/core/ErrorCode.h with only enum SWC::Error::Code
    added enum client::SQL:Cmd & Cmd recognize_cmd(int& err, const string& sql)
    added SWC::Thrift::Result and function Results exec_sql(1:string sql)
    added timestamp interval(one-side) to support Condition::NE in client::SQL
    moved Python pkg module swcdb.thrift.gen to swcdb.thrift.native
    added Python package with modules swcdb.thrift.{tornado,twisted,zopeif}
    added PyPi package 'swcdb', `pip install swcdb` available
    moved src/py/swcdb/pkg to src/py/package/swcdb
    extended doxygen with Doxyfile for each Doxyfile-[language].doxy
    added documentations to docs/ and configured Jekyll build
    changed from gh-wiki to documenaions on www.swcdb.org website & GitHub Pages
    fixed quit call in Ranger CompactRange::response on error

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.4.8...v0.4.9)
******





### [SWC-DB v0.4.8](https://github.com/kashirin-alex/swc-db/releases/tag/v0.4.8) (2020-09-15)

    added files COPYING.md & NOTICE.md
    updated Copyright notice format
    added FILES to install DESTINATION share/doc/swcdb
    added <license> to maven build pom.xml
    added install THIRD-PARTY_{Name}-bundled.jar.txt DESTINATION share/doc/swcdb

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.4.7...v0.4.8)
******





### [SWC-DB v0.4.7](https://github.com/kashirin-alex/swc-db/releases/tag/v0.4.7) (2020-09-14)

    added support for Ceph File System (libswcdb_fs_ceph)
    changed install only required headers
    moved src/cc/etc to src/etc
    added MutableVec source Cells/MutableVec.cc
    renamed MapMutable to MutableMap
    added KeyComparator.cc to target libswcdb
    changed MACRO SWC_CAN_INLINE without storage specifier
    added SWC::format(const char*,...) 'printf' format checking
    moved core/sys to common/ & removed target swcdb_core_sys
    added Thrift::SpecSchemas::patterns
    added examples/geospatial_analysis-xyz_and_props
    added flag DISPLAY_COLUMN to select SQL in shell SWC-DB(client)
    added Shell_DbClient.cc source file
    added Java package org.swcdb.thrift
    added Thrift Service sources for .NET Standard
    added Initial org.swcdb.jdbc.thrift Java package
    added class CompletionCounter<CountT>
    added size_t size_bytes_enc(bool) Ranger::{CellStore::{Read,Block::Read}}
    fixed chk_align instructions in Mutable::scan_version_multi
    improved Mutable::_narrow with consideration on max_revs
    added config option swc.rgr.ram.reserved.percent
    added function with registered mem usage to Resources class
    added LOG_WARN at Low-Memory state to class Resources
    changed set_option tcp::no_delay at ConnHandler constructor
    removed ConfigSSL::configure_{client,server} applied with make_{clt,srv}
    switched ConfigSSL::verify to asio::ssl::host_name_verification
    changed ConnHandlerSSL::handshake_{clt,srv} to handshake(handshake_type,.)
    added to configuration files default specialization of 'swc.logging.level'
    added Serialization::{encode,decode,length}_bytes
    removed Serialization::{encode,decode,length}_{str,vstr}
    optimized/fixed fletcher32 checksum 8-bit by 16-bit
    added fs-TYPE initial shell CLI & added 'ls'/'list' command
    added size_of_internal() to
      DB::Specs::{Interval,Key,Interval}
      DB::Cells::{MutableVec,Mutable,Interval,KeyVec}
      Ranger::Range::Block
    added Resources::mem_usage of actual Ranger::Column,Range,Req{Scan,Add}
    added reconnect to hdfs process in FS::HadoopJVM
    added ENOENT case in FS::Interface::rename with check exists
    added new class Buffer<> in core/Buffer.h and switched for usage
    removed DynamicBuffer and StaticBuffer classes and files
    changed Buffer::ensure by actual size expected to be required
    changed client::Query::Select response_partial call via client::default_io
    added test integration/utils/utils_shell_dbclient
    added mandatory __attribute__((optimize("-O3")) in DB::KeySeq
    changed Ranger Log::Compact max workers by half of hw_concurrency
    added separate Mutex for Ranger::Block::m_key_end
    added bool Ranger::Column/s::m_releasing checking
    added Ranger request profiling with block locate time
    added Ranger Block::ScanState
    change Ranger Blocks::scan preload Block only at prior Block QUEUED
    fixed remain buffer in DB::Cell::KeyVec::decode
    fixed SQL list-columns parse by combo CID/NAME/PATTERN
    added function Resources::available_{cpu_mhz, mem_mb}()
    added Protocol::Rgr::Params::ReportResRsp
    added Ranger report handler case for function RESOURCES
    added Manager get next-Ranger for range-assignment by Resources load_scale
    added Ranger shell CLI command "report-resources" & extended "report"
    added CommBuf::append_i8(uint8_t)
    added Protocol::Mngr::Command::REPORT & Manager ColumnStatus handler
    added db/Types files for Mngr-Column & Range-State
    added Manager report-function RANGERS_STATUS
    added functions ConnQueue::get_endpoint_{remote,local}()
    added Manager::MngrRole methods get_states & get_inchain_endpoint
    added Protocol::Params::Report::RspManagersStatus
    added request Protocol::Mngr::Req::ManagersStatus
    added Manager report handler case for function MANAGERS_STATUS
    added Manager shell CLI command 'status' for managers reporting
    fixed Manager standalone RANGERS role
    improved Managers roles executions & role changes
    fixed Ranger CommitLogCompact call 'finished_write' on error
    changed cases of asio::post on Env::IoCtx via Env::IoCtx::post
    changed Ranger asio::post to use RangerEnv via maintenance_post
    added Manager report handler case for function CLUSTER_STATUS
    added Manager shell CLI command "cluster_status"
    added swcdb_cluster task wait-ready
    added config options:
      swc.mngr.column.health.interval.check
      swc.mngr.column.health.checks
    added Manager Column Health Check
    changed Ranger REPORT Protocol to Function based requests
    removed ConnQueueReqBase::was_called & aligned event error incl. to types
    added Ranger::Callback::ColumnsUnloaded
    added source files for Ranger Column & Columns
    added class Protocol::Common::Params::ColumnsInterval
    added bool MngdColumns::active(cid_t& b, cid_t& e)
    added Protocol::Rgr::Command::COLUMNS_UNLOAD & Ranger Handler
    added Manager Protocol::Rgr::Req::ColumnsUnload
    added to Manager Ranger::State::MARKED_OFFLINE and a case change to ACK
    fixed Ranger CellStore::Read::release_fd behaviour
    fixed Manager's range-type handlers error rsp
    added Manager health-check of range for State::QUEUED
    deprecated use of Rgr::Req::AssignIdNeeded in class Manager::Rangers
    added ENOTEMPTY case for rename in FS::Interface, overwrite as file rename
    fixed MutableVec::split & added Mutable::can_split
    fixed Ranger Range Compact at small Cellstore or Block size
    fixed Ranger at stopped Range Compact skip 'request_again' to Mngr
    changed exceptions handling to any type with std::current_exception()
    updated Error Codes
    adjustments of Code Optimizations and Formattings
    deprecated & removed QueueRunnable in Fragment & CellStore::Read::Block
    added Fragments::_narrow & m_last_id (Fragment unique id state)
    updated for gcc-10.2
    added sources for /cli/Shell_{Manager,Ranger,Fs}.h
    added check of CompactRange::m_log_sz vs new-size
    added cc sources of fs/Broker/Protocol/{params,req}/
    changed serialization to vi32/64 in FS::Protocol::Params
    added necessary try blocks in FS::Protocol::Req for Params::decode
    added BlockLoader fragment loaded state cond
    added uint8_t CellStore::Block::Header::is_any (ANY_BEGIN,ANY_END)
    added possible early-split in CompactRange
    fixed SWC_LOG_OUT & reduced inlined-inst in MACROs (binaries less ~%1)
    deprecated most of to_string functions & added print(std::ostream&)
    added class Ranger::RangeSplit
    added immediate-split in CompactRange
    added cpu MHz in sys/Rsources.h fallback by /proc/cpuinfo
    added Compaction::m_compacting keep shared use-count +1 positive
    added cmake MACRO INSTALL_LIBS & cmake opt. SWC_INSTALL_DEP_LIBS=OFF/ON
    adjusted include "swcdb/core/Compat.h" first in inclusion tree
    added assure CompactRange::completion() done once
    added core/Malloc.h & definer option -DSWC_MALLOC_NOT_INSISTENT=OFF/ON
    fixed some fixes over -fanalyzer warnings
    changed encode & decode IPv4 Serialization in network byte order
    changed updates to ASIO DEPRECATED
    changed -fstd by CMAKE_C/CXX_STANDARD(11/20) & with ASIO_NO_DEPRECATED
    updated for latest libhdfspp apache-hadoop trunk

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.3.0...v0.4.7)
******







### [SWC-DB v0.3.0](https://github.com/kashirin-alex/swc-db/releases/tag/v0.3.0) (2020-07-07)

    released Initial debug version

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/d5ef0f442df2290496c5e4201659035ac9cf8222...v0.3.0)
***
