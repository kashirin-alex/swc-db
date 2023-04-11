/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fsbroker_AppContext_h
#define swcdb_fsbroker_AppContext_h

#include "swcdb/core/Serialization.h"

#include "swcdb/core/comm/AppContext.h"
#include "swcdb/core/comm/AppHandler.h"

#include "swcdb/db/client/Clients.h"
#include "swcdb/fs/Interface.h"
#include "swcdb/fsbroker/FsBrokerEnv.h"

#include "swcdb/common/Protocol/handlers/NotImplemented.h"
#include "swcdb/fsbroker/handlers/Exists.h"
#include "swcdb/fsbroker/handlers/Remove.h"
#include "swcdb/fsbroker/handlers/Length.h"
#include "swcdb/fsbroker/handlers/Mkdirs.h"
#include "swcdb/fsbroker/handlers/Readdir.h"
#include "swcdb/fsbroker/handlers/Rmdir.h"
#include "swcdb/fsbroker/handlers/Rename.h"
#include "swcdb/fsbroker/handlers/Write.h"
#include "swcdb/fsbroker/handlers/ReadAll.h"
#include "swcdb/fsbroker/handlers/CombiPread.h"
#include "swcdb/fsbroker/handlers/Create.h"
#include "swcdb/fsbroker/handlers/Append.h"
#include "swcdb/fsbroker/handlers/Open.h"
#include "swcdb/fsbroker/handlers/Read.h"
#include "swcdb/fsbroker/handlers/Pread.h"
#include "swcdb/fsbroker/handlers/Seek.h"
#include "swcdb/fsbroker/handlers/Flush.h"
#include "swcdb/fsbroker/handlers/Sync.h"
#include "swcdb/fsbroker/handlers/Close.h"


namespace SWC { namespace FsBroker {


class AppContext final : public Comm::AppContext {

  struct CommandHandler {
    // in-order of Protocol::FsBroker::Coomand
    static constexpr const Comm::AppHandler_t handlers[] = {
      &Comm::Protocol::Common::Handler::not_implemented,
      &Comm::Protocol::FsBroker::Handler::open,
      &Comm::Protocol::FsBroker::Handler::create,
      &Comm::Protocol::FsBroker::Handler::close,
      &Comm::Protocol::FsBroker::Handler::read,
      &Comm::Protocol::FsBroker::Handler::append,
      &Comm::Protocol::FsBroker::Handler::seek,
      &Comm::Protocol::FsBroker::Handler::remove,
      &Comm::Protocol::FsBroker::Handler::length,
      &Comm::Protocol::FsBroker::Handler::pread,
      &Comm::Protocol::FsBroker::Handler::mkdirs,
      &Comm::Protocol::FsBroker::Handler::flush,
      &Comm::Protocol::FsBroker::Handler::rmdir,
      &Comm::Protocol::FsBroker::Handler::readdir,
      &Comm::Protocol::FsBroker::Handler::exists,
      &Comm::Protocol::FsBroker::Handler::rename,
      &Comm::Protocol::FsBroker::Handler::sync,
      &Comm::Protocol::FsBroker::Handler::write,
      &Comm::Protocol::FsBroker::Handler::read_all,
      &Comm::Protocol::FsBroker::Handler::combi_pread

      //&Comm::Protocol::FsBroker::Handler::debug,
      //&Comm::Protocol::FsBroker::Handler::status,
      //&Comm::Protocol::FsBroker::Handler::shutdown
    };
    Comm::ConnHandlerPtr conn;
    Comm::Event::Ptr     ev;
    SWC_CAN_INLINE
    CommandHandler(const Comm::ConnHandlerPtr& a_conn,
                   const Comm::Event::Ptr& a_ev) noexcept
                  : conn(a_conn), ev(a_ev) {
      Env::FsBroker::in_process(1);
    }
    ~CommandHandler() noexcept { }
    void operator()() {
      if(!ev->expired() && conn->is_open())
        handlers[ev->header.command](conn, ev);
      Env::FsBroker::in_process(-1);
    }
  };


  public:

  typedef std::shared_ptr<AppContext> Ptr;

  AppContext()
      : Comm::AppContext(
          Env::Config::settings()->get<Config::Property::Value_enum_g>(
            "swc.FsBroker.comm.encoder")) {

    auto settings = Env::Config::settings();

    Env::IoCtx::init(
      settings->get_bool("swc.FsBroker.concurrency.relative"),
      settings->get_i32("swc.FsBroker.handlers")
    );

    auto fs_type = FS::fs_type(settings->get_str("swc.fs.broker.underlying"));
    SWC_ASSERT(fs_type != FS::Type::BROKER);
    Env::FsInterface::init(settings, fs_type);

    if(settings->get_bool("swc.FsBroker.metrics.enabled")) {
      Env::Clients::init(
        (settings->get_bool("swc.FsBroker.metrics.report.broker")
          ? client::Clients::make(
              *settings,
              Env::IoCtx::io(),
              client::ContextBroker::Ptr(
                new client::ContextBroker(*settings))
            )
          : client::Clients::make(
              *settings,
              Env::IoCtx::io(),
              client::ContextManager::Ptr(
                new client::ContextManager(*settings)),
              client::ContextRanger::Ptr(
                new client::ContextRanger(*settings))
            )
        )->init()
      );
    }
    Env::FsBroker::init();

    auto period = settings->get<Config::Property::Value_int32_g>(
      "swc.cfg.dyn.period");
    if(period->get()) {
      Env::IoCtx::io()->set_periodic_timer(
        period,
        []() noexcept { Env::Config::settings()->check_dynamic_files(); }
      );
    }
  }

  void init(const std::string& host,
            const Comm::EndPoints& endpoints) override {
    int sig = 0;
    Env::IoCtx::io()->set_signals();
    shutting_down(std::error_code(), sig);

    if((m_metrics = Env::FsBroker::metrics_track())) {
      m_metrics->configure_fsbroker(host.c_str(), endpoints);
      m_metrics->start();
    }
  }

  void set_srv(Comm::server::SerializedServer::Ptr srv){
    m_srv = srv;
  }

  virtual ~AppContext() noexcept { }

  void handle_established(Comm::ConnHandlerPtr conn) override {
    m_srv->connection_add(conn);
    if(m_metrics)
      m_metrics->net->connected(conn);
  }

  void handle_disconnect(Comm::ConnHandlerPtr conn) noexcept override {
    m_srv->connection_del(conn);
    if(m_metrics)
      m_metrics->net->disconnected(conn);
  }

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override {
    // SWC_LOG_OUT(LOG_DEBUG, ev->print(SWC_LOG_OSTREAM << "handle: "); );
    if(!Env::FsBroker::can_process())
      return conn->do_close();

    if(ev->error) {
      m_metrics->net->error(conn);

    } else if(!ev->header.command ||
               ev->header.command >= Comm::Protocol::FsBroker::MAX_CMD) {
      Comm::Protocol::Common::Handler::not_implemented(conn, ev);
      if(m_metrics)
        m_metrics->net->error(conn);

    } else {
      Env::IoCtx::post(CommandHandler(conn, ev));
      if(m_metrics)
        m_metrics->net->command(conn, ev->header.command);
    }

    Env::FsBroker::in_process(-1);
  }

  void net_bytes_sent(const Comm::ConnHandlerPtr& conn, size_t b)
                      noexcept override {
    if(m_metrics)
      m_metrics->net->sent(conn, b);
  }

  void net_bytes_received(const Comm::ConnHandlerPtr& conn, size_t b)
                          noexcept override {
    if(m_metrics)
      m_metrics->net->received(conn, b);
  }

  void net_accepted(const Comm::EndPoint& endpoint, bool secure)
                    noexcept override {
    if(m_metrics)
      m_metrics->net->accepted(endpoint, secure);
  }

  void shutting_down(const std::error_code &ec, const int &sig) {
    if(!sig) { // set signals listener
      struct Task {
        AppContext* ptr;
        SWC_CAN_INLINE
        Task(AppContext* a_ptr) noexcept : ptr(a_ptr) { }
        void operator()(const std::error_code& ec, const int &sig) {
          if(ec == asio::error::operation_aborted)
            return;
          SWC_LOGF(LOG_INFO, "Received signal, sig=%d ec=%s",
                    sig, ec.message().c_str());
          ptr->shutting_down(ec, sig);
        }
      };
      Env::IoCtx::io()->signals->async_wait(Task(this));

      SWC_LOGF(LOG_INFO, "Listening for Shutdown signal, set at sig=%d ec=%s",
              sig, ec.message().c_str());
      return;
    }

    SWC_LOGF(LOG_INFO, "Shutdown signal, sig=%d ec=%s", sig, ec.message().c_str());
    std::shared_ptr<std::thread> d(new std::thread);
    *d.get() = std::thread([d, ptr=shared_from_this()]{ ptr->stop(); });
    d->detach();
  }

  void stop() override {

    auto guard = m_srv->stop_accepting(); // no further requests accepted

    Env::FsBroker::shuttingdown();
    if(m_metrics) {
      #if defined(SWC_ENABLE_SANITIZER)
        m_metrics->wait();
      #endif
      Env::Clients::get()->stop_services();
    }

    Env::IoCtx::io()->stop();

    Env::FsInterface::interface()->stop();

    m_srv->shutdown();

    #if defined(SWC_ENABLE_SANITIZER)
      std::this_thread::sleep_for(std::chrono::seconds(2));
      m_srv = nullptr;
      m_metrics = nullptr;
      Env::FsBroker::reset();
      Env::Clients::reset();
      Env::FsInterface::reset();
      Env::IoCtx::reset();
    #endif

    guard = nullptr;
  }

  private:
  Comm::server::SerializedServer::Ptr m_srv = nullptr;
  Metric::Reporting::Ptr              m_metrics = nullptr;
};


}}



#endif // swcdb_fsbroker_AppContext_h
