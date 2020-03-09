/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_thriftbroker_AppContext_h
#define swc_app_thriftbroker_AppContext_h

#include "swcdb/thrift/gen-cpp/Broker.h"

#include "swcdb/db/client/Clients.h"
#include "swcdb/thrift/broker/AppContextClient.h"
#include "swcdb/thrift/broker/AppHandler.h"


namespace SWC { namespace Thrift {


class AppContext : virtual public BrokerIfFactory {
 public:
  
  AppContext() : m_run(true) { 
    auto settings = Env::Config::settings();
    Env::IoCtx::init(settings->get_i32("swc.ThriftBroker.handlers"));
    
    int sig = 0;
    Env::IoCtx::io()->set_signals();
    shutting_down(std::error_code(), sig);

    Env::Clients::init(
      std::make_shared<client::Clients>(
        Env::IoCtx::io()->shared(),
        std::make_shared<client::ThriftBroker::AppContext>()
      )
    );
    
    //Env::FsInterface::init(FS::fs_type(settings->get_str("swc.fs")));
    
    auto period = settings->get<Property::V_GINT32>("swc.cfg.dyn.period");
    if(period->get()) {
      Env::IoCtx::io()->set_periodic_timer(
        period,
        [](){Env::Config::settings()->check_dynamic_files();}
      );
    }
  }

  virtual ~AppContext() { }
  
  void wait_while_run() {
    std::unique_lock<std::mutex> lock_wait(m_mutex);
    m_cv.wait(lock_wait, [this]{return !m_run;});
  }

  BrokerIf* getHandler(const thrift::TConnectionInfo& connInfo) {
    return new AppHandler;
  }

  void releaseHandler(ServiceIf* hdlr) override {
    AppHandler* handler = dynamic_cast<AppHandler*>(hdlr);
    handler->disconnected();
    
    delete hdlr;
  }

  private:

  
  void shutting_down(const std::error_code &ec, const int &sig) {

    if(sig==0){ // set signals listener
      Env::IoCtx::io()->signals()->async_wait(
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

    stop();
  }

  void stop() {

    Env::Clients::get()->rgr_service->stop();
    Env::Clients::get()->mngr_service->stop();
    
    //Env::FsInterface::interface()->stop();
    Env::IoCtx::io()->stop();

    SWC_LOG(LOG_INFO, "Exit");
    {
      std::scoped_lock lock(m_mutex);
      m_run = false;
    }
    m_cv.notify_all();
    //std::quick_exit(0);
  }

  std::mutex                                    m_mutex;
  bool                                          m_run;
  std::condition_variable                       m_cv;
};



}}


//#ifdef SWC_IMPL_SOURCE
#include "swcdb/thrift/gen-cpp/Broker.cpp"
#include "swcdb/thrift/gen-cpp/Broker_constants.cpp"
#include "swcdb/thrift/gen-cpp/Broker_types.cpp"
#include "swcdb/thrift/gen-cpp/Service.cpp"
#include "swcdb/thrift/gen-cpp/Service_constants.cpp"
#include "swcdb/thrift/gen-cpp/Service_types.cpp"
//#include "swcdb/thrift/Converters.cc"
//#endif 


#endif // swc_app_thriftbroker_AppContext_h