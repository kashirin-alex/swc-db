# **SWC-DB©** - History/Changes Log
**All notable SWC-DB© changes and releases are documented in this file.** _- start 'watching' the master branch to receive updates_



***



### [SWC-DB master](https://github.com/kashirin-alex/swc-db/tree/master) (upcoming-release)

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

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.4.18...master)
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
