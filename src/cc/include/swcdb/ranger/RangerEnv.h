/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_RangerEnv_h
#define swcdb_ranger_RangerEnv_h

#include "swcdb/common/sys/Resources.h"
#include "swcdb/fs/Interface.h"
#include "swcdb/common/Files/RgrData.h"

#include "swcdb/db/client/Query/Select.h"
#include "swcdb/db/client/Query/Update.h"


namespace SWC {


namespace Ranger {
class Compaction;
class Columns;
}


namespace Env {

class Rgr final {
  public:

  static void init() {
    SWC_ASSERT(m_env == nullptr);
    m_env = std::make_shared<Rgr>();
  }

  static void start();

  static void shuttingdown();

  static void wait_if_in_process();

  static Common::Files::RgrData* rgr_data() {
    return &m_env->m_rgr_data;
  }

  static bool is_not_accepting() {
    return m_env->m_not_accepting;
  }

  static bool is_shuttingdown() {
    return m_env->m_shuttingdown;
  }

  static int64_t in_process() {
    return m_env->m_in_process;
  }

  static void in_process(int64_t count) {
    m_env->m_in_process += count;
  }

  static Rgr* get() {
    return m_env.get();
  }

  static Comm::IoContext* maintenance_io() {
    return m_env->mnt_io.get();
  }

  static Comm::IoContext::Ptr io() {
    return m_env->app_io;
  }

  template <typename T_Handler>
  SWC_CAN_INLINE
  static void post(T_Handler&& handler)  {
    m_env->app_io->post(handler);
  }

  template <typename T_Handler>
  SWC_CAN_INLINE
  static void maintenance_post(T_Handler&& handler)  {
    m_env->mnt_io->post(handler);
  }

  SWC_CAN_INLINE 
  static Common::Resources& res() {
    return m_env->_resources;
  }

  static bool compaction_available();

  static void compaction_schedule(uint32_t ms);
  
  static Ranger::Columns* columns() {
    return m_env->_columns;
  }

  static client::Query::Update* updater() {
    return m_env->_updater.get();
  }

  const SWC::Config::Property::V_GUINT8::Ptr      cfg_cs_max;
  const SWC::Config::Property::V_GINT32::Ptr      cfg_cs_sz;
  
  const SWC::Config::Property::V_GUINT8::Ptr      cfg_log_rollout_ratio;
  
  const SWC::Config::Property::V_GUINT8::Ptr      cfg_compact_percent;
  const SWC::Config::Property::V_GUINT8::Ptr      cfg_cs_replication;

  const SWC::Config::Property::V_GINT32::Ptr      cfg_blk_size;
  const SWC::Config::Property::V_GINT32::Ptr      cfg_blk_cells;
  const SWC::Config::Property::V_GENUM::Ptr       cfg_blk_enc;
  
  Comm::IoContext::Ptr        app_io;
  Comm::IoContext::Ptr        mnt_io;
  Ranger::Compaction*         _compaction;
  Ranger::Columns*            _columns;
  client::Query::Update::Ptr  _updater;
  Common::Resources           _resources;

  explicit Rgr();

  ~Rgr();

  private:
  inline static std::shared_ptr<Rgr>  m_env           = nullptr;

  Common::Files::RgrData              m_rgr_data;
  std::atomic<bool>                   m_shuttingdown     = false;
  std::atomic<bool>                   m_not_accepting    = false;
  std::atomic<int64_t>                m_in_process       = 0;
 
};


} // namespace Env

} // namespace SWC


#include "swcdb/ranger/db/Columns.h"
#include "swcdb/ranger/db/Compaction.h"



namespace SWC { namespace Env {

Rgr::Rgr() 
    : cfg_cs_max(
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GUINT8>(
          "swc.rgr.Range.CellStore.count.max")),
      cfg_cs_sz(
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GINT32>(
          "swc.rgr.Range.CellStore.size.max")),
      cfg_log_rollout_ratio(
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GUINT8>(
          "swc.rgr.Range.CommitLog.rollout.ratio")),
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
          SWC::Env::Config::settings()->get_i32("swc.rgr.handlers"))
      ),
      mnt_io(
        Comm::IoContext::make(
          "Maintenance", 
          SWC::Env::Config::settings()->get_i32("swc.rgr.maintenance.handlers"))
      ),
      _compaction(nullptr),
      _columns(new Ranger::Columns()),
      _updater(std::make_shared<client::Query::Update>()),  
      _resources(
        app_io->ptr(),
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GINT32>(
          "swc.rgr.ram.allowed.percent"),
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GINT32>(
          "swc.rgr.ram.reserved.percent"),
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GINT32>(
          "swc.rgr.ram.release.rate"),
        [this](size_t bytes) { return _columns->release(bytes); }) {
}

Rgr::~Rgr() {
  if(_compaction) 
    delete _compaction;
  delete _columns;
}

void Rgr::start() {
  SWC_ASSERT(m_env != nullptr);

  m_env->_compaction = new Ranger::Compaction();
  m_env->_compaction->schedule();
}

void Rgr::shuttingdown() {
  m_env->m_not_accepting = true;

  m_env->_compaction->stop();
  m_env->mnt_io->stop();
  
  m_env->_updater->commit();
  m_env->_updater->wait();
  
  m_env->_columns->unload_all(false);

  wait_if_in_process();

  m_env->m_shuttingdown = true;

  m_env->_resources.stop();
}

void Rgr::wait_if_in_process() {
  size_t n = 0;
  while(in_process()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    m_env->_columns->unload_all(true); //re-check
    if(++n % 10 == 0)
      SWC_LOGF(LOG_WARN, "In-process=%lu check=%lu", in_process(), n);
  }
}

bool Rgr::compaction_available() {
  return m_env->_compaction->available();
}

void Rgr::compaction_schedule(uint32_t ms) {
  if(m_env && m_env->_compaction)
    m_env->_compaction->schedule(ms);
}


}} // namespace SWC::Env


#endif // swcdb_ranger_RangerEnv_h
