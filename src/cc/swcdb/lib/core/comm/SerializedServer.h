/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_comm_SerializedServer_h
#define swc_core_comm_SerializedServer_h

#include <string>
#include <vector>
#include <memory>
#include <iostream>

#include "ConnHandler.h"


namespace SWC { namespace server {


class Acceptor{
  public:
  typedef std::shared_ptr<Acceptor> Ptr;

  Acceptor(asio::ip::tcp::acceptor& acceptor, 
           AppContext::Ptr app_ctx, IOCtxPtr io_ctx)
          : m_acceptor(std::move(acceptor)), m_app_ctx(app_ctx), 
            m_io_ctx(io_ctx) {
    do_accept();
    
    HT_INFOF("Listening On: [%s]:%d fd=%d", 
             m_acceptor.local_endpoint().address().to_string().c_str(), 
             m_acceptor.local_endpoint().port(), 
             (ssize_t)m_acceptor.native_handle());
  }

  void stop() {
    HT_INFOF("Stopping to Listen On: [%s]:%d fd=%d", 
             m_acceptor.local_endpoint().address().to_string().c_str(), 
             m_acceptor.local_endpoint().port(), 
             (ssize_t)m_acceptor.native_handle());

    if(m_acceptor.is_open())
      m_acceptor.close();
  }

  virtual ~Acceptor(){}

  asio::ip::tcp::acceptor* sock() {
    return &m_acceptor;
  }

  private:
  
  void do_accept() {
    m_acceptor.async_accept(
      [this](std::error_code ec, asio::ip::tcp::socket new_sock) {
        if(ec) {
          if(ec.value() != 125) 
            HT_DEBUGF("SRV-accept error=%d(%s)", 
                      ec.value(), ec.message().c_str());
          return;
        }

        auto conn = std::make_shared<ConnHandler>(m_app_ctx, new_sock);
        conn->new_connection();
        conn->accept_requests();

        do_accept();
      }
    );
  }

  private:
  asio::ip::tcp::acceptor m_acceptor;
  AppContext::Ptr         m_app_ctx;
  IOCtxPtr                m_io_ctx;
};



class SerializedServer {
  public:

  typedef std::shared_ptr<SerializedServer> Ptr;

  SerializedServer(
    std::string name, 
    uint32_t reactors, uint32_t workers,
    std::string port_cfg_name,
    AppContext::Ptr app_ctx
  ): m_appname(name), m_run(true){
    
    HT_INFOF("STARTING SERVER: %s, reactors=%d, workers=%d", 
              m_appname.c_str(), reactors, workers);

    SWC::PropertiesPtr props = Env::Config::settings()->properties;

    Strings addrs = props->has("addr") ? props->get<Strings>("addr") : Strings();
    String host;
    if(props->has("host"))
      host = host.append(props->get<String>("host"));
    else {
      char hostname[256];
      gethostname(hostname, sizeof(hostname));
      host.append(hostname);
    }
    
    EndPoints endpoints = Resolver::get_endpoints(
      props->get<int32_t>(port_cfg_name),
      addrs,
      host,
      true
    );

    EndPoints endpoints_final;

    for(uint32_t reactor=0; reactor<reactors; reactor++) {

      auto io_ctx = std::make_shared<asio::io_context>(workers);
      m_wrk.push_back(asio::make_work_guard(*io_ctx.get()));

      for (std::size_t i = 0; i < endpoints.size(); ++i) {
        auto& endpoint = endpoints[i];

        if(reactor == 0) { 
          auto acceptor = asio::ip::tcp::acceptor(*io_ctx.get(), endpoint);
          m_acceptors.push_back(
            std::make_shared<Acceptor>(acceptor, app_ctx, io_ctx));
          endpoints_final.push_back(m_acceptors[i]->sock()->local_endpoint());

        } else {
          auto acceptor = asio::ip::tcp::acceptor(*io_ctx.get(), 
            endpoint.protocol(), dup(m_acceptors[i]->sock()->native_handle()));
          m_acceptors.push_back(
            std::make_shared<Acceptor>(acceptor, app_ctx, io_ctx));
        }
      }

      if(reactor == 0)
        app_ctx->init(endpoints_final);

      asio::thread_pool* pool = new asio::thread_pool(workers);
      for(int n=0; n<workers; n++)
        asio::post(*pool, [d=io_ctx, run=&m_run]{
          // HT_INFOF("START DELAY: %s 3secs",  m_appname.c_str());
          std::this_thread::sleep_for(std::chrono::milliseconds(5000));
          do{
            d->run();
            d->restart();
          }while(run->load());
        });
      m_thread_pools.push_back(pool);

    }

  }

  void run(){
    for(;;) {
      auto it = m_thread_pools.begin();
      if(it == m_thread_pools.end())
        break;
      (*it)->join();
      delete *it;
      m_thread_pools.erase(it);
    }
      
    HT_INFOF("STOPPED SERVER: %s", m_appname.c_str());
  }

  void stop_accepting() {
    for(;;) {
      auto it = m_acceptors.begin();
      if(it == m_acceptors.end())
        break;
      (*it)->stop();
      m_acceptors.erase(it);
    }

    HT_INFOF("STOPPED ACCEPTING: %s", m_appname.c_str());
  }

  void shutdown() {
    HT_INFOF("STOPPING SERVER: %s", m_appname.c_str());
    m_run.store(false);

    {
      std::lock_guard<std::mutex> lock(m_mutex);
      for(auto& conn : m_conns)
        conn->close();
    }
    for (std::size_t i = 0; i < m_wrk.size(); ++i)
      m_wrk[i].reset();
      //m_wrk[i].get_executor().context().stop();
  }

  void connection_add(ConnHandlerPtr conn) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_conns.push_back(conn);

    //HT_DEBUGF("%s, conn-add open=%d", m_appname.c_str(), m_conns.size());
  }

  void connection_del(ConnHandlerPtr conn) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto it=m_conns.begin(); it<m_conns.end(); it++) {
      if(conn->endpoint_remote == (*it)->endpoint_remote){
        m_conns.erase(it);
        break;
      }
    }
    //HT_DEBUGF("%s, conn-del open=%d", m_appname.c_str(), m_conns.size());
  }

  virtual ~SerializedServer(){}

  private:
  
  std::vector<asio::thread_pool*> m_thread_pools;
  std::atomic<bool>               m_run;
  std::string                     m_appname;
  std::vector<Acceptor::Ptr>      m_acceptors;
  std::vector<asio::executor_work_guard<asio::io_context::executor_type>> m_wrk;

  std::mutex                  m_mutex;
  std::vector<ConnHandlerPtr> m_conns;
};

}}

#endif // swc_core_comm_SerializedServer_h