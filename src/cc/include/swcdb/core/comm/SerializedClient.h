/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_comm_SerializedClient_h
#define swc_core_comm_SerializedClient_h

#include <chrono>
#include <string>
#include <queue>
#include <unordered_map>

#include "swcdb/core/comm/IoContext.h"
#include "swcdb/core/comm/ConnHandler.h"


namespace SWC { namespace client {

struct SSL_Context final {
  std::string     subject_name;
  std::string     ca;
  std::vector<asio::ip::network_v4> nets_v4;
  std::vector<asio::ip::network_v6> nets_v6;
  
  asio::ssl::context create();

  void load_ca(const std::string& ca_filepath);

};


class ServerConnections : public std::enable_shared_from_this<ServerConnections> {
  public:
  typedef std::shared_ptr<ServerConnections>    Ptr;
  typedef std::function<void(ConnHandlerPtr)> NewCb_t;

  ServerConnections(const std::string& srv_name, const EndPoint& endpoint,
                    IOCtxPtr ioctx, AppContext::Ptr ctx, SSL_Context* ssl_ctx);
  
  virtual ~ServerConnections();

  void reusable(ConnHandlerPtr &conn, bool preserve);

  void connection(ConnHandlerPtr &conn, std::chrono::milliseconds timeout, 
                  bool preserve);
  
  void connection(std::chrono::milliseconds timeout, NewCb_t cb, 
                  bool preserve);

  void put_back(ConnHandlerPtr conn);
  
  const bool empty();

  void close_all();

  private:

  const std::string             m_srv_name;
  const EndPoint                m_endpoint;
  IOCtxPtr                      m_ioctx;
  AppContext::Ptr               m_ctx;
  std::mutex                    m_mutex;
  std::queue<ConnHandlerPtr>    m_conns;
  SSL_Context*                  m_ssl_ctx;
};


class Serialized : public std::enable_shared_from_this<Serialized> {

  public:

  typedef std::shared_ptr<Serialized>                  Ptr;
  typedef std::unordered_map<size_t, ServerConnections::Ptr> Map;

  Serialized(const std::string& srv_name, IOCtxPtr ioctx, AppContext::Ptr ctx);

  ServerConnections::Ptr get_srv(EndPoint endpoint);

  ConnHandlerPtr get_connection(
        const EndPoints& endpoints, 
        std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
        uint32_t probes=0,
        bool preserve=false
  );

  void get_connection(
        const EndPoints& endpoints, 
        ServerConnections::NewCb_t cb,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
        uint32_t probes=0,
        bool preserve=false
  );
  
  void get_connection(
        const EndPoints& endpoints, 
        ServerConnections::NewCb_t cb,
        std::chrono::milliseconds timeout,
        uint32_t probes, uint32_t tries, 
        int next,
        bool preserve=false
  );

  void preserve(ConnHandlerPtr conn);

  void close(ConnHandlerPtr conn);

  IOCtxPtr io();        
  
  const std::string to_str(ConnHandlerPtr conn);
  
  void stop();

  virtual ~Serialized();

  private:
  const std::string     m_srv_name;
  IOCtxPtr              m_ioctx;
  AppContext::Ptr       m_ctx;

  const bool            m_use_ssl;
  SSL_Context*          m_ssl_ctx;

  std::mutex            m_mutex;
  Map                   m_srv_conns;
  std::atomic<bool>     m_run;

};


}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/SerializedClient.cc"
#endif 

#endif // swc_core_comm_SerializedClient_h