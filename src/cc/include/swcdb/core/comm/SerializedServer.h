/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
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


namespace SWC { namespace Comm { namespace server {


class Acceptor : protected asio::ip::tcp::acceptor {
  public:
  typedef std::shared_ptr<Acceptor> Ptr;

  Acceptor(asio::ip::tcp::acceptor& acceptor, 
           AppContext::Ptr& app_ctx, bool is_plain=true);

  void stop();

  ~Acceptor();

  asio::ip::tcp::acceptor* sock();

  protected:

  AppContext::Ptr         m_app_ctx;

  private:

  void do_accept();

};


class AcceptorSSL final : public Acceptor {
  public:

  AcceptorSSL(asio::ip::tcp::acceptor& acceptor, 
              AppContext::Ptr& app_ctx, ConfigSSL* ssl_cfg);

  ~AcceptorSSL();

  private:
  
  void do_accept();

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

  void connection_add(const ConnHandlerPtr& conn);

  void connection_del(const ConnHandlerPtr& conn);

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


}}} //namespace SWC::Comm::server



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/SerializedServer.cc"
#endif 

#endif // swc_core_comm_SerializedServer_h