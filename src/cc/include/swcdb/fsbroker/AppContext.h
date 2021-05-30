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

  public:

  AppContext()
      : Comm::AppContext(
          Env::Config::settings()->get<Config::Property::V_GENUM>(
            "swc.FsBroker.comm.encoder")) {

    auto settings = Env::Config::settings();

    Env::IoCtx::init(settings->get_i32("swc.FsBroker.handlers"));

    auto fs_type = FS::fs_type(settings->get_str("swc.fs.broker.underlying"));
    SWC_ASSERT(fs_type != FS::Type::BROKER);
    Env::FsInterface::init(settings, fs_type);

    if(settings->get_bool("swc.FsBroker.metrics.enabled")) {
      Env::Clients::init(
        (settings->get_bool("swc.FsBroker.metrics.report.broker")
          ? std::make_shared<client::Clients>(
              *settings,
              Env::IoCtx::io(),
              std::make_shared<client::ContextBroker>(*settings)
            )
          : std::make_shared<client::Clients>(
              *settings,
              Env::IoCtx::io(),
              std::make_shared<client::ContextManager>(*settings),
              std::make_shared<client::ContextRanger>(*settings)
            )
        )->init()
      );
    }
    Env::FsBroker::init();

    auto period = settings->get<Config::Property::V_GINT32>(
      "swc.cfg.dyn.period");
    if(period->get()) {
      Env::IoCtx::io()->set_periodic_timer(
        period,
        [](){Env::Config::settings()->check_dynamic_files();}
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

  virtual ~AppContext() { }

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override {
    // SWC_LOG_OUT(LOG_DEBUG, ev->print(SWC_LOG_OSTREAM << "handle: "); );
    if(!Env::FsBroker::can_process())
      return conn->do_close();

    switch (ev->type) {

      case Comm::Event::Type::ESTABLISHED:
        m_srv->connection_add(conn);
        if(m_metrics)
          m_metrics->net->connected(conn);
        break;

      case Comm::Event::Type::DISCONNECT:
        m_srv->connection_del(conn);
        if(m_metrics)
          m_metrics->net->disconnected(conn);
        break;

      case Comm::Event::Type::ERROR:
        if(m_metrics)
          m_metrics->net->error(conn);
        break;

      case Comm::Event::Type::MESSAGE: {
        if(!ev->header.command ||
            ev->header.command >= Comm::Protocol::FsBroker::MAX_CMD) {
          Comm::Protocol::Common::Handler::not_implemented(conn, ev);
          if(m_metrics)
            m_metrics->net->error(conn);
        } else {
          Env::FsBroker::in_process(1);
          Env::IoCtx::post([conn, ev]() {
            if(!ev->expired() && conn->is_open())
              handlers[ev->header.command](conn, ev);
            Env::FsBroker::in_process(-1);
          });
          if(m_metrics)
            m_metrics->net->command(conn, ev->header.command);
        }
        break;
      }

      default:
        SWC_LOGF(LOG_WARN, "Unimplemented event-type (%d)", int(ev->type));
        if(m_metrics)
          m_metrics->net->error(conn);
        break;

    }
    Env::FsBroker::in_process(-1);
  }

  void net_bytes_sent(const Comm::ConnHandlerPtr& conn, size_t b) override {
    if(m_metrics)
      m_metrics->net->sent(conn, b);
  }

  void net_bytes_received(const Comm::ConnHandlerPtr& conn, size_t b) override {
    if(m_metrics)
      m_metrics->net->received(conn, b);
  }

  void accepted(const Comm::EndPoint& endpoint, bool secure) override {
    if(m_metrics)
      m_metrics->net->accepted(endpoint, secure);
  }

  void shutting_down(const std::error_code &ec, const int &sig) {
    if(!sig) { // set signals listener
      Env::IoCtx::io()->signals->async_wait(
        [ptr=this](const std::error_code &ec, const int &sig){
          SWC_LOGF(LOG_INFO, "Received signal, sig=%d ec=%s", sig, ec.message().c_str());
          ptr->shutting_down(ec, sig);
        }
      );
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
    if(m_metrics)
      Env::Clients::get()->stop();

    Env::IoCtx::io()->stop();

    Env::FsInterface::interface()->stop();

    m_srv->shutdown();

    #if defined(SWC_ENABLE_SANITIZER)
      std::this_thread::sleep_for(std::chrono::seconds(2));
      m_srv = nullptr;
      if(m_metrics) {
        m_metrics->wait();
        m_metrics = nullptr;
      }
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
