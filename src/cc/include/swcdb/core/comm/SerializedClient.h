/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_comm_SerializedClient_h
#define swcdb_core_comm_SerializedClient_h

#include <chrono>
#include <string>
#include <queue>
#include <unordered_map>

#include "swcdb/core/QueueSafe.h"
#include "swcdb/core/comm/IoContext.h"
#include "swcdb/core/comm/ConnHandler.h"
#include "swcdb/core/comm/ConfigSSL.h"


namespace SWC { namespace Comm { 


//! The SWC-DB client C++ namespace 'SWC::Comm::client'
namespace client {



class ServerConnections final : 
      private Core::QueueSafe<ConnHandlerPtr>, 
      public std::enable_shared_from_this<ServerConnections> {

  public:

  typedef std::shared_ptr<ServerConnections>          Ptr;
  typedef std::function<void(const ConnHandlerPtr&)>  NewCb_t;

  using Core::QueueSafe<ConnHandlerPtr>::empty;
  using Core::QueueSafe<ConnHandlerPtr>::push;

  ServerConnections(const std::string& srv_name, const EndPoint& endpoint,
                    const IOCtxPtr& ioctx, const AppContext::Ptr& ctx, 
                    ConfigSSL* ssl_cfg);
  
 ~ServerConnections();

  void reusable(ConnHandlerPtr& conn, bool preserve);

  void connection(ConnHandlerPtr& conn, 
                  const std::chrono::milliseconds& timeout, 
                  bool preserve);
  
  void connection(const std::chrono::milliseconds& timeout, 
                  const NewCb_t& cb, bool preserve);

  void close_all();

  private:

  const std::string             m_srv_name;
  const EndPoint                m_endpoint;
  IOCtxPtr                      m_ioctx;
  AppContext::Ptr               m_ctx;
  ConfigSSL*                    m_ssl_cfg;
};


class Serialized final : 
      private std::unordered_map<size_t, ServerConnections::Ptr>,
      public std::enable_shared_from_this<Serialized> {

  public:

  typedef std::shared_ptr<Serialized>                        Ptr;

  Serialized(const std::string& srv_name, 
             const IOCtxPtr& ioctx, 
             const AppContext::Ptr& ctx);

  ServerConnections::Ptr get_srv(const EndPoint& endpoint);

  ConnHandlerPtr get_connection(
        const EndPoints& endpoints, 
        const std::chrono::milliseconds& timeout=std::chrono::milliseconds(0),
        uint32_t probes=0,
        bool preserve=false
  ) noexcept;

  void get_connection(
        const EndPoints& endpoints, 
        const ServerConnections::NewCb_t& cb,
        const std::chrono::milliseconds& timeout=std::chrono::milliseconds(0),
        uint32_t probes=0,
        bool preserve=false
  ) noexcept;

  void preserve(ConnHandlerPtr& conn);

  void close(ConnHandlerPtr& conn);

  IOCtxPtr io();        
  
  void print(std::ostream& out, ConnHandlerPtr& conn);
  
  void stop();

  virtual ~Serialized();

  private:
  
  ConnHandlerPtr _get_connection(
        const EndPoints& endpoints, 
        const std::chrono::milliseconds& timeout=std::chrono::milliseconds(0),
        uint32_t probes=0,
        bool preserve=false
  );
  
  void _get_connection(
        const EndPoints& endpoints, 
        const ServerConnections::NewCb_t& cb,
        const std::chrono::milliseconds& timeout,
        uint32_t probes, uint32_t tries, 
        size_t next,
        bool preserve=false
  );

  const std::string     m_srv_name;
  IOCtxPtr              m_ioctx;
  AppContext::Ptr       m_ctx;

  const bool            m_use_ssl;
  ConfigSSL*            m_ssl_cfg;

  Core::MutexSptd       m_mutex;
  std::atomic<bool>     m_run;

};


}}} // namespace SWC::Comm::client


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/SerializedClient.cc"
#endif 

#endif // swcdb_core_comm_SerializedClient_h