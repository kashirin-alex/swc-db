/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_RangerEnv_h
#define swcdb_ranger_RangerEnv_h


#include "swcdb/common/sys/Resources.h"
#include "swcdb/fs/Interface.h"
#include "swcdb/common/Files/RgrData.h"

#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/client/Query/Select/Scanner.h"
#include "swcdb/db/client/Query/Update/Committer.h"
#include "swcdb/db/client/Query/Update/Handlers/Common.h"
#include "swcdb/ranger/queries/update/MetricsReporting.h"


namespace SWC {


namespace Ranger {
class Compaction;
class Columns;
}


namespace Env {

class Rgr final {
  public:

  static void init() {
    SWC_ASSERT(!m_env);
    m_env = std::make_shared<Rgr>();
  }

  static void start();

  static void shuttingdown();

  static void wait_if_in_process();

  static Common::Files::RgrData* rgr_data() noexcept {
    return &m_env->m_rgr_data;
  }

  static bool is_not_accepting() noexcept {
    return m_env->m_not_accepting;
  }

  static bool is_shuttingdown() noexcept {
    return m_env->m_shuttingdown;
  }

  static int64_t in_process() noexcept {
    return m_env->m_in_process;
  }

  static void in_process(int64_t count) noexcept {
    m_env->m_in_process.fetch_add(count);
  }

  static size_t scan_reserved_bytes() noexcept {
    return m_env->m_scan_reserved_bytes;
  }

  static void scan_reserved_bytes_add(uint32_t bytes) noexcept {
    m_env->m_scan_reserved_bytes.fetch_add(bytes);
  }

  static void scan_reserved_bytes_sub(uint32_t bytes) noexcept {
    m_env->m_scan_reserved_bytes.fetch_sub(bytes);
  }

  static Rgr* get() noexcept {
    return m_env.get();
  }

  static Comm::IoContextPtr maintenance_io() noexcept {
    return m_env->mnt_io;
  }

  static Comm::IoContextPtr io() noexcept {
    return m_env->app_io;
  }

  template <typename T_Handler>
  SWC_CAN_INLINE
  static void post(T_Handler&& handler)  {
    m_env->app_io->post(std::move(handler));
  }

  template <typename T_Handler>
  SWC_CAN_INLINE
  static void maintenance_post(T_Handler&& handler)  {
    m_env->mnt_io->post(std::move(handler));
  }

  SWC_CAN_INLINE
  static System::Resources& res() noexcept {
    return m_env->_resources;
  }

  static bool log_compact_possible() noexcept;

  static void log_compact_finished() noexcept;

  static bool compaction_available() noexcept;

  static void compaction_schedule(uint32_t ms);

  static Ranger::Columns* columns() noexcept {
    return m_env->_columns;
  }

  static client::Query::Update::Handlers::Common* updater() noexcept {
    return m_env->_update_hdlr.get();
  }

  static Ranger::Metric::Reporting::Ptr& metrics_track() noexcept {
    return m_env->_reporting;
  }

  static void reset() noexcept {
    m_env = nullptr;
  }

  const SWC::Config::Property::V_GUINT8::Ptr  cfg_req_add_concurrency;

  const SWC::Config::Property::V_GUINT8::Ptr  cfg_cs_max;
  const SWC::Config::Property::V_GINT32::Ptr  cfg_cs_sz;

  const SWC::Config::Property::V_GUINT8::Ptr  cfg_log_rollout_ratio;
  const SWC::Config::Property::V_GUINT8::Ptr  cfg_log_compact_cointervaling;
  const SWC::Config::Property::V_GUINT8::Ptr  cfg_log_fragment_preload;

  const SWC::Config::Property::V_GUINT8::Ptr  cfg_compact_percent;
  const SWC::Config::Property::V_GUINT8::Ptr  cfg_cs_replication;

  const SWC::Config::Property::V_GINT32::Ptr  cfg_blk_size;
  const SWC::Config::Property::V_GINT32::Ptr  cfg_blk_cells;
  const SWC::Config::Property::V_GENUM::Ptr   cfg_blk_enc;

  Comm::IoContextPtr                            app_io;
  Comm::IoContextPtr                            mnt_io;
  Ranger::Compaction*                           _compaction;
  Ranger::Columns*                              _columns;
  client::Query::Update::Handlers::Common::Ptr  _update_hdlr;
  Ranger::Metric::Reporting::Ptr                _reporting;
  System::Resources                             _resources;

  explicit Rgr();

  ~Rgr();

  private:
  inline static std::shared_ptr<Rgr>  m_env = nullptr;

  Common::Files::RgrData              m_rgr_data;
  Core::AtomicBool                    m_shuttingdown;
  Core::AtomicBool                    m_not_accepting;
  Core::Atomic<int64_t>               m_in_process;
  Core::Atomic<size_t>                m_scan_reserved_bytes;

};


} // namespace Env

} // namespace SWC


#include "swcdb/ranger/db/Columns.h"
#include "swcdb/ranger/db/Compaction.h"



namespace SWC { namespace Env {

Rgr::Rgr()
    : cfg_req_add_concurrency(
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GUINT8>(
          "swc.rgr.Range.req.update.concurrency")),
      cfg_cs_max(
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GUINT8>(
          "swc.rgr.Range.CellStore.count.max")),
      cfg_cs_sz(
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GINT32>(
          "swc.rgr.Range.CellStore.size.max")),
      cfg_log_rollout_ratio(
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GUINT8>(
          "swc.rgr.Range.CommitLog.rollout.ratio")),
      cfg_log_compact_cointervaling(
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GUINT8>(
          "swc.rgr.Range.CommitLog.Compact.cointervaling")),
      cfg_log_fragment_preload(
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GUINT8>(
          "swc.rgr.Range.CommitLog.Fragment.preload")),
      cfg_compact_percent(
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GUINT8>(
          "swc.rgr.Range.compaction.percent")),
      cfg_cs_replication(
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GUINT8>(
          "swc.rgr.Range.CellStore.replication")),
      cfg_blk_size(
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GINT32>(
          "swc.rgr.Range.block.size")),
      cfg_blk_cells(
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GINT32>(
          "swc.rgr.Range.block.cells")),
      cfg_blk_enc(
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GENUM>(
          "swc.rgr.Range.block.encoding")),
      app_io(
        Comm::IoContext::make(
          "Ranger",
          SWC::Env::Config::settings()->get_i32(
            "swc.rgr.handlers")
        )
      ),
      mnt_io(
        Comm::IoContext::make(
          "Maintenance",
          SWC::Env::Config::settings()->get_i32(
            "swc.rgr.maintenance.handlers")
        )
      ),
      _compaction(nullptr),
      _columns(new Ranger::Columns()),
      _update_hdlr(
        client::Query::Update::Handlers::Common::make(
          Env::Clients::get(), nullptr, app_io)),
      _reporting(
        SWC::Env::Config::settings()->get_bool("swc.rgr.metrics.enabled")
          ? std::make_shared<Ranger::Metric::Reporting>(app_io)
          : nullptr
      ),
      _resources(
        app_io,
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GINT32>(
          "swc.rgr.ram.allowed.percent"),
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GINT32>(
          "swc.rgr.ram.reserved.percent"),
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GINT32>(
          "swc.rgr.ram.release.rate"),
        _reporting ? &_reporting->system : nullptr,
        [this](size_t bytes) { return _columns->release(bytes); }
      ),
      m_shuttingdown(false), m_not_accepting(false),
      m_in_process(0), m_scan_reserved_bytes(0) {
}

Rgr::~Rgr() {
  if(_compaction)
    delete _compaction;
  delete _columns;
}

void Rgr::start() {
  m_env->_compaction = new Ranger::Compaction();
  m_env->_compaction->schedule();
}

void Rgr::shuttingdown() {
  m_env->m_not_accepting.store(true);

  if(m_env->_reporting)
    m_env->_reporting->stop();

  if(m_env->_compaction)
    m_env->_compaction->stop();
  m_env->mnt_io->stop();

  m_env->_update_hdlr->commit_if_need();
  m_env->_update_hdlr->wait();

  m_env->_columns->unload_all(false);

  wait_if_in_process();

  m_env->m_shuttingdown.store(true);

  m_env->_resources.stop();
}

void Rgr::wait_if_in_process() {
  size_t n = 0;
  while(in_process()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    m_env->_columns->unload_all(true); //re-check
    if(!(++n % 10))
      SWC_LOGF(LOG_WARN, "In-process=%ld check=%lu", in_process(), n);
  }
}

bool Rgr::log_compact_possible() noexcept {
  return m_env->_compaction->log_compact_possible();
}

void Rgr::log_compact_finished() noexcept {
  return m_env->_compaction->log_compact_finished();
}

bool Rgr::compaction_available() noexcept {
  return m_env->_compaction->available();
}

void Rgr::compaction_schedule(uint32_t ms) {
  if(m_env && m_env->_compaction)
    m_env->_compaction->schedule(ms);
}


}} // namespace SWC::Env


#include "swcdb/ranger/queries/update/MetricsReporting.cc"


#endif // swcdb_ranger_RangerEnv_h
