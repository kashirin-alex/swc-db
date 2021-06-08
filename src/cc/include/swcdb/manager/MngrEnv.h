/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_MngrEnv_h
#define swcdb_manager_MngrEnv_h


#include "swcdb/common/sys/Resources.h"

#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/Columns/Schemas.h"

#include "swcdb/manager/db/Columns.h"
#include "swcdb/manager/MngrRole.h"
#include "swcdb/manager/Rangers.h"
#include "swcdb/manager/MngdColumns.h"
#include "swcdb/manager/queries/update/MetricsReporting.h"


namespace SWC { namespace Env {

class Mngr final {
  public:

  static void init(const Comm::EndPoints& endpoints) {
    SWC_ASSERT(!m_env);
    m_env.reset(new Mngr(endpoints));
  }

  static Comm::IoContextPtr io() noexcept {
    return m_env->app_io;
  }

  template <typename T_Handler>
  SWC_CAN_INLINE
  static void post(T_Handler&& handler)  {
    m_env->app_io->post(std::move(handler));
  }

  SWC_CAN_INLINE
  static Manager::Metric::Reporting::Ptr& metrics_track() noexcept {
    return m_env->_reporting;
  }

  SWC_CAN_INLINE
  static System::Resources& res() noexcept {
    return m_env->_resources;
  }

  static DB::Schemas* schemas() noexcept {
    return &m_env->m_schemas;
  }

  static Manager::Columns* columns() noexcept {
    return &m_env->m_columns;
  }

  static Manager::MngrRole* role() noexcept {
    return &m_env->m_role;
  }

  static Manager::Rangers* rangers() noexcept {
    return &m_env->m_rangers;
  }

  static Manager::MngdColumns* mngd_columns() noexcept {
    return &m_env->m_mngd_columns;
  }

  static void reset() noexcept {
    m_env = nullptr;
  }

  static void stop();


  Mngr(const Comm::EndPoints& endpoints)
      : app_io(
          Comm::IoContext::make(
            "Manager",
            SWC::Env::Config::settings()->get_i32("swc.mngr.handlers")
          )
        ),
        cfg_ram_percent_allowed(100, nullptr),
        cfg_ram_percent_reserved(0, nullptr),
        cfg_ram_release_rate(100, nullptr),
        _reporting(
          SWC::Env::Config::settings()->get_bool("swc.mngr.metrics.enabled")
            ? new Manager::Metric::Reporting(app_io)
            : nullptr
        ),
        _resources(
          app_io,
          &cfg_ram_percent_allowed,
          &cfg_ram_percent_reserved,
          &cfg_ram_release_rate,
          _reporting ? &_reporting->system : nullptr,
          nullptr
        ),
        m_role(app_io, endpoints),
        m_rangers(app_io) {
  }

  //~Mngr() { }

  Comm::IoContextPtr                  app_io;

  SWC::Config::Property::V_GINT32     cfg_ram_percent_allowed;
  SWC::Config::Property::V_GINT32     cfg_ram_percent_reserved;
  SWC::Config::Property::V_GINT32     cfg_ram_release_rate;
  private:

  inline static std::shared_ptr<Mngr> m_env = nullptr;
  Manager::Metric::Reporting::Ptr     _reporting;
  System::Resources                   _resources;
  DB::Schemas                         m_schemas;
  Manager::Columns                    m_columns;
  Manager::MngrRole                   m_role;
  Manager::Rangers                    m_rangers;
  Manager::MngdColumns                m_mngd_columns;

};


}} // SWC::Env namespace

#include "swcdb/manager/MngrRole.cc"
#include "swcdb/manager/Rangers.cc"
#include "swcdb/manager/MngdColumns.cc"
#include "swcdb/manager/ColumnHealthCheck.cc"
#include "swcdb/manager/queries/update/MetricsReporting.cc"



namespace SWC { namespace Env {

void Mngr::stop() {
  if(m_env->_reporting)
    m_env->_reporting->stop();

  m_env->m_role.stop();
}

}}


#endif // swcdb_manager_MngrEnv_h
