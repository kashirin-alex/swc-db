/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_thrift_broker_ThriftBrokerEnv_h
#define swcdb_thrift_broker_ThriftBrokerEnv_h


#include "swcdb/common/sys/Resources.h"
#include "swcdb/thrift/broker/queries/update/MetricsReporting.h"


namespace SWC { namespace Env {

class ThriftBroker final {
  public:

  static void init() {
    SWC_ASSERT(!m_env);
    m_env = std::make_shared<ThriftBroker>();
  }

  SWC_CAN_INLINE
  static SWC::ThriftBroker::Metric::Reporting::Ptr& metrics_track() noexcept {
    return m_env->_reporting;
  }

  SWC_CAN_INLINE
  static Common::Resources& res() noexcept {
    return m_env->_resources;
  }

  static void reset() noexcept {
    m_env = nullptr;
  }

  static void stop() {
    if(m_env->_reporting)
      m_env->_reporting->stop();
  }


  ThriftBroker()
      : cfg_ram_percent_allowed(100, nullptr),
        cfg_ram_percent_reserved(0, nullptr),
        cfg_ram_release_rate(100, nullptr),
        _reporting(
          SWC::Env::Config::settings()->get_bool("swc.ThriftBroker.metrics.enabled")
            ? std::make_shared<SWC::ThriftBroker::Metric::Reporting>(
                Env::IoCtx::io(),
                SWC::Env::Config::settings()
                  ->get<SWC::Config::Property::V_GINT32>(
                    "swc.ThriftBroker.metrics.report.interval"))
            : nullptr
        ),
        _resources(
          Env::IoCtx::io(),
          &cfg_ram_percent_allowed,
          &cfg_ram_percent_reserved,
          &cfg_ram_release_rate,
          nullptr,
          _reporting ? &_reporting->hardware : nullptr
        ) {
  }

  SWC::Config::Property::V_GINT32     cfg_ram_percent_allowed;
  SWC::Config::Property::V_GINT32     cfg_ram_percent_reserved;
  SWC::Config::Property::V_GINT32     cfg_ram_release_rate;
  private:

  inline static std::shared_ptr<ThriftBroker> m_env = nullptr;
  SWC::ThriftBroker::Metric::Reporting::Ptr   _reporting;
  Common::Resources                           _resources;

};


}} // SWC::Env namespace


#endif // swcdb_thrift_broker_ThriftBrokerEnv_h
