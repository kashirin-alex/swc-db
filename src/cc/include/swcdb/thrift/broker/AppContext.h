/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_app_thriftbroker_AppContext_h
#define swcdb_app_thriftbroker_AppContext_h


#include "swcdb/db/client/Clients.h"
#include "swcdb/thrift/broker/AppHandler.h"


namespace SWC {


namespace client {
//! The SWC-DB ThriftBroker's Client to Database C++ namespace 'SWC::client::ThriftBroker'
namespace ThriftBroker { }
}


namespace ThriftBroker {


class AppContext final : virtual public BrokerIfFactory {
 public:

  AppContext() : m_run(true) {
    auto settings = Env::Config::settings();
    Env::IoCtx::init(settings->get_i32("swc.ThriftBroker.clients.handlers"));

    int sig = 0;
    Env::IoCtx::io()->set_signals();
    shutting_down(std::error_code(), sig);

    Env::Clients::init(
      std::make_shared<client::Clients>(
        Env::IoCtx::io(),
        std::make_shared<client::ContextManager>(),
        std::make_shared<client::ContextRanger>()
      )
    );

    //Env::FsInterface::init(FS::fs_type(settings->get_str("swc.fs")));

    auto period = settings->get<Config::Property::V_GINT32>(
      "swc.cfg.dyn.period");
    if(period->get()) {
      Env::IoCtx::io()->set_periodic_timer(
        period,
        [](){Env::Config::settings()->check_dynamic_files();}
      );
    }
  }

  virtual ~AppContext() { }

  void wait_while_run() {
    std::unique_lock lock_wait(m_mutex);
    m_cv.wait(lock_wait, [this]{return !m_run;});
  }

  BrokerIf* getHandler(const thrift::TConnectionInfo& connInfo) override {
    AppHandler* handler = new AppHandler(
      std::dynamic_pointer_cast<thrift::transport::TSocket>(
        connInfo.transport));
    if(handler->socket) try {
      SWC_LOG_OUT(LOG_INFO,
        SWC_LOG_OSTREAM << "Connection Opened(hdlr=" << size_t(handler)
                        << " [" << handler->socket->getPeerAddress() << "]:"
                        << handler->socket->getPeerPort()
                        << ") open=" << m_connections.increment_and_count();
      );
    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
    }
    return handler;
  }

  void releaseHandler(ServiceIf* hdlr) override {
    AppHandler* handler = dynamic_cast<AppHandler*>(hdlr);
    handler->disconnected();

    if(handler->socket) try {
      SWC_LOG_OUT(LOG_INFO,
        SWC_LOG_OSTREAM << "Connection Closed(hdlr=" << size_t(handler)
                        << " [" << handler->socket->getPeerAddress() << "]:"
                        << handler->socket->getPeerPort()
                        << ") open=" << m_connections.decrement_and_count();
      );
    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
    }
    delete handler;
  }

  void shutting_down(const std::error_code& ec, const int& sig) {
    if(!sig) { // set signals listener
      Env::IoCtx::io()->signals->async_wait(
        [this](const std::error_code& ec, const int &sig) {
          if(ec == asio::error::operation_aborted)
            return;
          SWC_LOGF(LOG_INFO, "Received signal, sig=%d ec=%s", sig, ec.message().c_str());
          shutting_down(ec, sig);
        }
      );
      SWC_LOGF(LOG_INFO, "Listening for Shutdown signal, set at sig=%d ec=%s",
              sig, ec.message().c_str());
      return;
    } else {

      bool at = true;
      if(!m_run.compare_exchange_weak(at, false))
        return;
    }

    SWC_LOGF(LOG_INFO, "Shutdown signal, sig=%d ec=%s", sig, ec.message().c_str());

    stop();
  }

  private:

  void stop() {

    Env::Clients::get()->rgr->stop();
    Env::Clients::get()->mngr->stop();
    Env::IoCtx::io()->stop();

    //Env::FsInterface::interface()->stop();

    std::scoped_lock lock(m_mutex);
    m_cv.notify_all();
  }

  std::mutex                                    m_mutex;
  Core::AtomicBool                              m_run;
  std::condition_variable                       m_cv;
  Core::CompletionCounter<size_t>               m_connections;
};



}}


#include "swcdb/thrift/gen-cpp/Broker.cpp"
#ifdef SWC_IMPL_SOURCE
#include "swcdb/thrift/gen-cpp/Service.cpp"
#include "swcdb/thrift/gen-cpp/Service_types.cpp"
//#include "swcdb/thrift/Converters.cc"
#endif


#endif // swcdb_app_thriftbroker_AppContext_h
