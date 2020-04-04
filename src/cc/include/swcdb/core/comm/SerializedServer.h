/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_core_comm_SerializedServer_h
#define swc_core_comm_SerializedServer_h

#include <string>
#include <vector>
#include <memory>
#include <iostream>

#include "swcdb/core/comm/IoContext.h"
#include "swcdb/core/comm/ConnHandler.h"
#include "swcdb/core/comm/ConfigSSL.h"


namespace SWC { namespace server {


class Acceptor {
  public:
  typedef std::shared_ptr<Acceptor> Ptr;

  Acceptor(asio::ip::tcp::acceptor& acceptor, 
           AppContext::Ptr app_ctx, IOCtxPtr io_ctx, 
           bool is_plain=true);

  void stop();

  virtual ~Acceptor();

  asio::ip::tcp::acceptor* sock();

  protected:
  asio::ip::tcp::acceptor m_acceptor;
  AppContext::Ptr         m_app_ctx;

  private:
  virtual void do_accept();

  IOCtxPtr                m_io_ctx;
};


class AcceptorSSL : public Acceptor {
  public:

  AcceptorSSL(asio::ip::tcp::acceptor& acceptor, 
              AppContext::Ptr app_ctx, IOCtxPtr io_ctx, 
              ConfigSSL* ssl_cfg);

  ~AcceptorSSL();

  private:
  
  void do_accept() override;

  ConfigSSL* m_ssl_cfg;
};


class SerializedServer final {
  public:

  typedef std::shared_ptr<SerializedServer> Ptr;

  SerializedServer(
    std::string name, 
    uint32_t reactors, uint32_t workers,
    const std::string& port_cfg_name,
    AppContext::Ptr app_ctx
  );

  void run();

  void stop_accepting();

  void shutdown();

  void connection_add(ConnHandlerPtr conn);

  void connection_del(ConnHandlerPtr conn);

  ~SerializedServer();

  private:
  
  std::vector<asio::thread_pool*> m_thread_pools;
  std::string                     m_appname;
  std::atomic<bool>               m_run;
  std::vector<Acceptor::Ptr>      m_acceptors;
  std::vector<asio::executor_work_guard<asio::io_context::executor_type>> m_wrk;

  Mutex                       m_mutex;
  std::vector<ConnHandlerPtr> m_conns;
  ConfigSSL*                  m_ssl_cfg;
};

}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/SerializedServer.cc"
#endif 

#endif // swc_core_comm_SerializedServer_h