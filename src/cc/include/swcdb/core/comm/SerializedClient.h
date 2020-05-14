/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_core_comm_SerializedClient_h
#define swc_core_comm_SerializedClient_h

#include <chrono>
#include <string>
#include <queue>
#include <unordered_map>

#include "swcdb/core/QueueSafe.h"
#include "swcdb/core/comm/IoContext.h"
#include "swcdb/core/comm/ConnHandler.h"
#include "swcdb/core/comm/ConfigSSL.h"


namespace SWC { namespace client {


class ServerConnections : 
      private QueueSafe<ConnHandlerPtr>, 
      public std::enable_shared_from_this<ServerConnections> {

  public:

  typedef std::shared_ptr<ServerConnections>          Ptr;
  typedef std::function<void(const ConnHandlerPtr&)>  NewCb_t;

  using QueueSafe<ConnHandlerPtr>::empty;
  using QueueSafe<ConnHandlerPtr>::push;

  ServerConnections(const std::string& srv_name, const EndPoint& endpoint,
                    const IOCtxPtr& ioctx, const AppContext::Ptr& ctx, 
                    ConfigSSL* ssl_cfg);
  
  virtual ~ServerConnections();

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


class Serialized : 
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
  );

  void get_connection(
        const EndPoints& endpoints, 
        const ServerConnections::NewCb_t& cb,
        const std::chrono::milliseconds& timeout=std::chrono::milliseconds(0),
        uint32_t probes=0,
        bool preserve=false
  );
  
  void get_connection(
        const EndPoints& endpoints, 
        const ServerConnections::NewCb_t& cb,
        const std::chrono::milliseconds& timeout,
        uint32_t probes, uint32_t tries, 
        int next,
        bool preserve=false
  );

  void preserve(ConnHandlerPtr& conn);

  void close(ConnHandlerPtr& conn);

  IOCtxPtr io();        
  
  std::string to_str(ConnHandlerPtr& conn);
  
  void stop();

  virtual ~Serialized();

  private:
  const std::string     m_srv_name;
  IOCtxPtr              m_ioctx;
  AppContext::Ptr       m_ctx;

  const bool            m_use_ssl;
  ConfigSSL*            m_ssl_cfg;

  Mutex                 m_mutex;
  std::atomic<bool>     m_run;

};


}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/SerializedClient.cc"
#endif 

#endif // swc_core_comm_SerializedClient_h