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

  static void init(System::Mem::ReleaseCall_t&& release_call) {
    SWC_ASSERT(!m_env);
    m_env.reset(new ThriftBroker(std::move(release_call)));
  }

  SWC_CAN_INLINE
  static SWC::ThriftBroker::Metric::Reporting::Ptr& metrics_track() noexcept {
    return m_env->_reporting;
  }

  SWC_CAN_INLINE
  static System::Resources& res() noexcept {
    return m_env->_resources;
  }

  static void reset() noexcept {
    m_env = nullptr;
  }

  static void stop() {
    if(m_env->_reporting)
      m_env->_reporting->stop();
  }


  ThriftBroker(System::Mem::ReleaseCall_t&& release_call)
      : _reporting(
          SWC::Env::Config::settings()->get_bool("swc.ThriftBroker.metrics.enabled")
            ? new SWC::ThriftBroker::Metric::Reporting(
                Env::IoCtx::io(),
                SWC::Env::Config::settings()
                  ->get<SWC::Config::Property::Value_int32_g>(
                    "swc.ThriftBroker.metrics.report.interval"))
            : nullptr
        ),
        _resources(
          Env::IoCtx::io(),
          SWC::Env::Config::settings()
            ->get<SWC::Config::Property::Value_int32_g>(
              "swc.ThriftBroker.ram.allowed.percent"),
          SWC::Env::Config::settings()
            ->get<SWC::Config::Property::Value_int32_g>(
              "swc.ThriftBroker.ram.reserved.percent"),
          SWC::Env::Config::settings()
            ->get<SWC::Config::Property::Value_int32_g>(
              "swc.ThriftBroker.ram.release.rate"),
          _reporting ? &_reporting->system : nullptr,
          std::move(release_call)
        ) {
  }

  ~ThriftBroker() noexcept { }

  private:

  inline static std::shared_ptr<ThriftBroker> m_env = nullptr;
  SWC::ThriftBroker::Metric::Reporting::Ptr   _reporting;
  System::Resources                           _resources;

};


}} // SWC::Env namespace



#include "swcdb/thrift/broker/queries/update/MetricsReporting.cc"


#endif // swcdb_thrift_broker_ThriftBrokerEnv_h
