/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_broker_BrokerEnv_h
#define swcdb_broker_BrokerEnv_h


#include "swcdb/common/sys/Resources.h"

#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/client/Query/Select/Scanner.h"
#include "swcdb/db/client/Query/Update/Committer.h"
#include "swcdb/db/client/Query/Update/Handlers/Common.h"
#include "swcdb/broker/queries/update/MetricsReporting.h"


namespace SWC { namespace Env {

class Bkr final {
  public:

  static void init() {
    SWC_ASSERT(!m_env);
    m_env = std::make_shared<Bkr>();
  }

  static void start();

  static void shuttingdown();

  static void wait_if_in_process();

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

  static Bkr* get() noexcept {
    return m_env.get();
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
  static System::Resources& res() noexcept {
    return m_env->_resources;
  }

  static Broker::Metric::Reporting::Ptr& metrics_track() noexcept {
    return m_env->_reporting;
  }

  static void reset() noexcept {
    m_env = nullptr;
  }

  Comm::IoContextPtr                    app_io;
  Broker::Metric::Reporting::Ptr       _reporting;
  System::Resources                    _resources;

  explicit Bkr();

  ~Bkr();

  private:
  inline static std::shared_ptr<Bkr>   m_env = nullptr;

  Core::AtomicBool                     m_shuttingdown;
  Core::AtomicBool                     m_not_accepting;
  Core::Atomic<int64_t>                m_in_process;

};


}} // namespace SWC::Env




namespace SWC { namespace Env {

Bkr::Bkr()
    : app_io(
        Comm::IoContext::make(
          "Broker",
          SWC::Env::Config::settings()->get_i32(
            "swc.bkr.handlers"))
      ),
      _reporting(
        SWC::Env::Config::settings()->get_bool("swc.bkr.metrics.enabled")
          ? std::make_shared<Broker::Metric::Reporting>(
              app_io,
              SWC::Env::Config::settings()
                ->get<SWC::Config::Property::V_GINT32>(
                  "swc.bkr.metrics.report.interval"))
          : nullptr
      ),
      _resources(
        app_io,
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GINT32>(
          "swc.bkr.ram.allowed.percent"),
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GINT32>(
          "swc.bkr.ram.reserved.percent"),
        SWC::Env::Config::settings()->get<SWC::Config::Property::V_GINT32>(
          "swc.bkr.ram.release.rate"),
        _reporting ? &_reporting->system : nullptr,
        [this](size_t bytes) noexcept { (void)this; (void)bytes; return 0; }
      ),
      m_shuttingdown(false), m_not_accepting(false),
      m_in_process(0) {
}

Bkr::~Bkr() {
}

void Bkr::start() {
}

void Bkr::shuttingdown() {
  m_env->m_not_accepting.store(true);

  if(m_env->_reporting)
    m_env->_reporting->stop();

  wait_if_in_process();

  m_env->m_shuttingdown.store(true);

  m_env->_resources.stop();
}

void Bkr::wait_if_in_process() {
  size_t n = 0;
  while(in_process()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    if(!(++n % 10))
      SWC_LOGF(LOG_WARN, "In-process=%ld check=%lu", in_process(), n);
  }
}


}} // namespace SWC::Env


#include "swcdb/broker/queries/update/MetricsReporting.cc"


#endif // swcdb_broker_BrokerEnv_h
