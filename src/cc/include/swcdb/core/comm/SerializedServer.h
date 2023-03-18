/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_comm_SerializedServer_h
#define swcdb_core_comm_SerializedServer_h


#include "swcdb/core/comm/IoContext.h"
#include "swcdb/core/comm/ConnHandler.h"
#include "swcdb/core/comm/ConfigSSL.h"


namespace SWC { namespace Comm {


//! The SWC-DB server C++ namespace 'SWC::Comm::server'
namespace server {



class Acceptor final : protected asio::ip::tcp::acceptor {
  public:
  typedef std::shared_ptr<Acceptor> Ptr;

  Acceptor(asio::ip::tcp::acceptor& acceptor,
           AppContext::Ptr& app_ctx,
           ConfigSSL* ssl_cfg);

  Acceptor(Acceptor&&)                 = delete;
  Acceptor(const Acceptor&)            = delete;
  Acceptor& operator=(Acceptor&&)      = delete;
  Acceptor& operator=(const Acceptor&) = delete;

  void stop();

  ~Acceptor() noexcept { }

  SWC_CAN_INLINE
  asio::ip::tcp::acceptor* sock() noexcept {
    return this;
  }

  private:
  struct Mixed;
  struct Plain;

  AppContext::Ptr   m_app_ctx;
  ConfigSSL*        m_ssl_cfg;

};



class SerializedServer final {
  public:

  typedef std::shared_ptr<SerializedServer> Ptr;

  SerializedServer(
    const Config::Settings& settings,
    std::string&& name,
    uint32_t reactors, uint32_t workers, uint16_t port,
    AppContext::Ptr app_ctx
  );

  SerializedServer(SerializedServer&&)                 = delete;
  SerializedServer(const SerializedServer&)            = delete;
  SerializedServer& operator=(SerializedServer&&)      = delete;
  SerializedServer& operator=(const SerializedServer&) = delete;

  void run();

  std::shared_ptr<IoContext::ExecutorWorkGuard> stop_accepting();

  void shutdown();

  void connection_add(const ConnHandlerPtr& conn);

  void connection_del(const ConnHandlerPtr& conn);

  ~SerializedServer() noexcept;

  private:

  const std::string               m_appname;
  Core::AtomicBool                m_run;
  Core::Vector<IoContextPtr>      m_io_contexts;
  Core::Vector<Acceptor::Ptr>     m_acceptors;

  Core::MutexSptd                 m_mutex;
  Core::Vector<ConnHandlerPtr>    m_conns;
  ConfigSSL*                      m_ssl_cfg;
};


}}} //namespace SWC::Comm::server



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/SerializedServer.cc"
#endif

#endif // swcdb_core_comm_SerializedServer_h
