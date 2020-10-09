# **SWC-DB©** - History/Changes Log
**All notable SWC-DB© changes and releases are documented in this file.** _- start 'watching' the master branch to receive updates_


***



### [SWC-DB master](https://github.com/kashirin-alex/swc-db/tree/master) (upcoming-release)

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
    added struct Buffer as sub-class of Comm::Header
    changed buffers details storage in class Comm::Header to Buffer type
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

[_Full Changelog_](https://github.com/kashirin-alex/swc-db/compare/v0.4.10...master)
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
    moved Python pkg module swcdb.thirft.gen to swcdb.thirft.native
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
