/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_thriftbroker_AppContext_h
#define swc_app_thriftbroker_AppContext_h

#include "swcdb/client/Clients.h"
#include "swcdb/thriftbroker/AppContextClient.h"

#include "swcdb/thrift/gen-cpp/Broker.h"

namespace SWC { 
namespace thrift = apache::thrift;
namespace Thrift {





class AppHandler : virtual public BrokerIf {
  public:

  const std::string addr;
  
  AppHandler(const std::string& addr) 
            : addr(addr) {
  }

  virtual ~AppHandler() { }



};







class AppContext : virtual public BrokerIfFactory {
 public:
  
  AppContext() { 
    Env::IoCtx::init(
      Env::Config::settings()->get<int32_t>("swc.ThriftBroker.handlers"));
    
    int sig = 0;
    Env::IoCtx::io()->set_signals();
    shutting_down(std::error_code(), sig);

    Env::Clients::init(
      std::make_shared<client::Clients>(
        Env::IoCtx::io()->shared(),
        std::make_shared<client::ThriftBroker::AppContext>()
      )
    );
    
    //Env::FsInterface::init(FS::fs_type(
    //  Env::Config::settings()->get<std::string>("swc.fs")));
  }

  virtual ~AppContext() { }
  

  BrokerIf* getHandler(const thrift::TConnectionInfo& connInfo) {
    auto sock = std::dynamic_pointer_cast<thrift::transport::TSocket>(connInfo.transport);
    const std::string addr = sock->getPeerAddress(); // +port
    
    std::scoped_lock lock(m_mutex);
    auto it = m_clients.find(addr);
    if(it != m_clients.end())
      return it->second;

    AppHandler* handler = new AppHandler(addr);
    m_clients.insert(std::make_pair(addr, handler));
    return handler;
  }

  void releaseHandler(ServiceIf* hdlr) override {
    AppHandler* handler = dynamic_cast<AppHandler*>(hdlr);
    std::scoped_lock lock(m_mutex);
    auto it = m_clients.find(handler->addr);
    if(it != m_clients.end())
      m_clients.erase(it);
    delete handler;
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
    
    Env::IoCtx::io()->stop();
    //Env::FsInterface::interface()->stop();
    //m_srv->shutdown();

    SWC_LOG(LOG_INFO, "Exit");
    std::quick_exit(0);
  }

  std::mutex                                    m_mutex;
  std::unordered_map<std::string, AppHandler*>  m_clients;
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