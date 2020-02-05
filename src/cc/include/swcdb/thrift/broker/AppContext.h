/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_thriftbroker_AppContext_h
#define swc_app_thriftbroker_AppContext_h

#include "swcdb/client/Clients.h"
#include "swcdb/thrift/broker/AppContextClient.h"

#include "swcdb/thrift/gen-cpp/Broker.h"

namespace SWC { 
namespace thrift = apache::thrift;
namespace Thrift {





class AppHandler : virtual public BrokerIf {
  public:

  const std::string host;
  const std::string addr;
  
  AppHandler(const std::string& host, const std::string& addr) 
            : host(host), addr(addr) {
  }

  virtual ~AppHandler() { }



};







class AppContext : virtual public BrokerIfFactory {
 public:
  
  AppContext() : m_run(true) { 
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
  
  void wait_while_run() {
    std::unique_lock<std::mutex> lock_wait(m_mutex);
    m_cv.wait(lock_wait, [this]{return !m_run;});
  }

  void add_host(const std::string& addr) {
    m_hosts.insert(std::make_pair(addr, Clients()));
  }

  BrokerIf* getHandler(const thrift::TConnectionInfo& connInfo) {
    auto sock = std::dynamic_pointer_cast<thrift::transport::TSocket>(connInfo.transport);
    
    auto& clients = m_hosts[sock->getHost()];
    const std::string addr = sock->getPeerAddress(); // +port

    std::scoped_lock lock(m_mutex);
    auto it = clients.find(addr);
    if(it != clients.end())
      return it->second;

    AppHandler* handler = new AppHandler(sock->getHost(), addr);
    clients.insert(std::make_pair(addr, handler));
    return handler;
  }

  void releaseHandler(ServiceIf* hdlr) override {
    AppHandler* handler = dynamic_cast<AppHandler*>(hdlr);
    auto& clients = m_hosts[handler->host];

    std::scoped_lock lock(m_mutex);
    auto it = clients.find(handler->addr);
    if(it != clients.end())
      clients.erase(it);
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
    //std::quick_exit(0);
    {
      std::scoped_lock lock(m_mutex);
      m_run = false;
    }
    m_cv.notify_all();
  }

  std::mutex                                    m_mutex;
  bool                                          m_run;
  typedef std::unordered_map<std::string, AppHandler*> Clients;
  std::unordered_map<std::string, Clients>      m_hosts;
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