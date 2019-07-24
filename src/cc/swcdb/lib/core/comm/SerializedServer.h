/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_comm_SerializedServer_h
#define swc_core_comm_SerializedServer_h

#include <string>
#include <vector>
#include <memory>
#include <iostream>


#include "AppContext.h"
#include "ConnHandlerServer.h"


namespace SWC { namespace server {



class Acceptor{
public:
  Acceptor(std::shared_ptr<asio::ip::tcp::acceptor> acceptor, 
           AppContextPtr app_ctx, IOCtxPtr io_ctx)
          : m_acceptor(acceptor), m_app_ctx(app_ctx), 
            m_io_ctx(io_ctx), m_run(true)
  {
    do_accept();
    
    HT_INFOF("Listening On: [%s]:%d fd=%d", 
             m_acceptor->local_endpoint().address().to_string().c_str(), 
             m_acceptor->local_endpoint().port(), 
             (size_t)m_acceptor->native_handle());

  }
  void stop(){
    HT_INFOF("Stopped Listening On: [%s]:%d fd=%d", 
             m_acceptor->local_endpoint().address().to_string().c_str(), 
             m_acceptor->local_endpoint().port(), 
             (size_t)m_acceptor->native_handle());

    m_run.store(false);
  }
  virtual ~Acceptor(){
    stop();
  }

private:
  void do_accept() {
    m_acceptor->async_accept(
      [this](std::error_code ec, asio::ip::tcp::socket new_sock) {
        if (!ec){
          (new ConnHandlerServer(
            m_app_ctx, 
            std::make_shared<asio::ip::tcp::socket>(std::move(new_sock)),
            m_io_ctx)
          )->new_connection();
        }
        if(m_run.load())
          do_accept();
      }
    );
  }

  private:
  std::shared_ptr<asio::ip::tcp::acceptor> m_acceptor;
  AppContextPtr     m_app_ctx;
  std::atomic<bool> m_run;
  IOCtxPtr          m_io_ctx;
};
typedef std::shared_ptr<Acceptor> AcceptorPtr;



class SerializedServer{
  public:

  SerializedServer(
    std::string name, 
    uint32_t reactors, uint32_t workers,
    std::string port_cfg_name,
    AppContextPtr app_ctx,
    bool detached=false
  ): m_appname(name), m_run(true){
    
    HT_INFOF("STARTING SERVER: %s, reactors=%d, workers=%d", 
              m_appname.c_str(), reactors, workers);

    SWC::PropertiesPtr props = SWC::Config::settings->properties;

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

    std::vector<std::shared_ptr<asio::ip::tcp::acceptor>> main_acceptors;
    EndPoints endpoints_final;

    for(uint32_t reactor=0;reactor<reactors;reactor++){

      std::shared_ptr<asio::io_context> io_ctx = 
        std::make_shared<asio::io_context>(workers);
      m_wrk.push_back(asio::make_work_guard(*io_ctx.get()));

      for (std::size_t i = 0; i < endpoints.size(); ++i){
        auto endpoint = endpoints[i];

        std::shared_ptr<asio::ip::tcp::acceptor> acceptor;
        if(reactor == 0){ 
          acceptor = std::make_shared<asio::ip::tcp::acceptor>(
            *io_ctx.get(), 
            endpoint
          );
          main_acceptors.push_back(acceptor);

        } else {
          acceptor = std::make_shared<asio::ip::tcp::acceptor>(
            *io_ctx.get(), 
            endpoint.protocol(),  
            dup(main_acceptors[i]->native_handle())
          );
        }
        
        m_acceptors.push_back(
          std::make_shared<Acceptor>(acceptor, app_ctx, io_ctx));

        if(reactor == 0){ 
          if(!acceptor->local_endpoint().address().to_string().compare("::"))
            endpoints_final.push_back(acceptor->local_endpoint());
          else {
          // + localhost public ips
            endpoints_final.push_back(acceptor->local_endpoint());
          }
        }
      }

      std::thread* t =  new std::thread(
        [this, d=io_ctx]{ 
          do{
            d->run();
            HT_DEBUG("SRV IO stopped, restarting");
            d->restart();
          }while(m_run.load());
          HT_DEBUG("SRV IO exited");
        });
      detached ? t->detach(): threads.push_back(t);
    }

    app_ctx->init(endpoints_final);

  }

  void run(){
    for (std::size_t i = 0; i < threads.size(); i++)
      threads[i]->join();
  }

  void stop() {
    m_run.store(false);
    for (std::size_t i = 0; i < m_acceptors.size(); ++i){
      m_acceptors[i]->stop();
    }

    for (std::size_t i = 0; i < m_wrk.size(); ++i){
      m_wrk[i].reset();
    }
    for (std::size_t i = 0; i < m_wrk.size(); ++i){
      m_wrk[i].get_executor().context().stop();
    }

    HT_INFOF("STOPPED SERVER: %s", m_appname.c_str());
  }

  
  virtual ~SerializedServer(){
    stop();
  }

  private:
  
  std::vector<std::thread*> threads;
  std::atomic<bool> m_run;
  std::string       m_appname;
  std::vector<AcceptorPtr> m_acceptors;
  std::vector<asio::executor_work_guard<asio::io_context::executor_type>> m_wrk;
};

typedef std::shared_ptr<SerializedServer> SerializedServerPtr;
}}

#endif // swc_core_comm_SerializedServer_h