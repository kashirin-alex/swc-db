/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_comm_SerializedClient_h
#define swc_core_comm_SerializedClient_h

#include <chrono>
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

#include <asio.hpp>

#include "swcdb/lib/core/comm/Resolver.h"
#include "AppContext.h"
#include "ConnHandlerClient.h"


namespace SWC { namespace client {

typedef ConnHandlerClientPtr ClientConPtr;

static std::string to_string(ClientConPtr con){
  std::string s("Connection");

  if(!con->is_open()) {
    s.append(" CLOSED");
    return s;
  }

  s.append(" remote=");
  try{
    s.append(con->endpoint_remote_str());
    s.append(" (hash=");
    s.append(std::to_string(con->endpoint_remote_hash()));
    s.append(")");
  } catch(...){
    s.append("Exception");
  }
  try{
    s.append(" local=");
    s.append(con->endpoint_local_str());
    s.append(" (hash=");
    s.append(std::to_string(con->endpoint_local_hash()));
    s.append(")");
  } catch(...){
    s.append("Exception");
  }
  return s;
}
  

class ServerConnections {
  public:

  ServerConnections(std::string srv_name, asio::ip::tcp::endpoint endpoint,
                    IOCtxPtr ioctx, AppContextPtr ctx)
                  : m_srv_name(srv_name), m_endpoints({endpoint}), 
                    m_ioctx(ioctx), m_ctx(ctx){}
  virtual ~ServerConnections(){}

  void connection(ClientConPtr &con,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0)){
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(!m_conns.empty()){
        con = m_conns.back();
        m_conns.pop_back();
      }
    }
    if (con != nullptr){
      if(con->is_open())
        HT_DEBUGF("Reusing connection to Service: %s, %s", 
                  m_srv_name.c_str(), to_string(con).c_str());
      else 
        con = nullptr;
    }

    if (con == nullptr){
        HT_DEBUGF("Connecting to Service: %s, addr=[%s]:%d", 
                  m_srv_name.c_str(), 
                  m_endpoints[0].address().to_string().c_str(), 
                  m_endpoints[0].port());
      try{
        SocketPtr s = std::make_shared<asio::ip::tcp::socket>(*m_ioctx.get());
        asio::connect(*s.get(), m_endpoints);
        con = std::make_shared<ConnHandlerClient>(m_ctx, s, m_ioctx);
        con->new_connection();
       
        HT_DEBUGF("New connection to Service: %s, %s", 
                  m_srv_name.c_str(), to_string(con).c_str());
      } catch(...){}
    }
  }

  void put_back(ClientConPtr con){
    std::lock_guard<std::mutex> lock(m_mutex);
    m_conns.push_back(con);
  }
  
  bool empty(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_conns.empty();
  }

  private:

  AppContextPtr   m_ctx;
  IOCtxPtr        m_ioctx;
  std::mutex      m_mutex;
  EndPoints       m_endpoints;
  std::string     m_srv_name;
  std::vector<ClientConPtr> m_conns;
};

typedef std::shared_ptr<ServerConnections> ServerConnectionsPtr;
typedef std::unordered_map<size_t, ServerConnectionsPtr> ServerConnectionsMap;

class SerializedClient{

  public:

  SerializedClient(
    std::string srv_name, 
    IOCtxPtr ioctx,
    AppContextPtr ctx
  ): m_srv_name(srv_name), m_ctx(ctx),
     m_run(true), 
     m_srv_conns(std::make_shared<ServerConnectionsMap>()),
     m_ioctx(ioctx)
  {
    HT_INFOF("Init to Service: %s", m_srv_name.c_str());
  }

  ClientConPtr get_connection(
        EndPoints endpoints, 
        std::chrono::milliseconds timeout = std::chrono::milliseconds(0),
        uint32_t probes=0
        ){
    
    ClientConPtr con = nullptr;
    if(endpoints.empty()){
      HT_WARNF("get_connection to Service: %s, Empty-Endpoints", m_srv_name.c_str());
      return con;
    }

    uint32_t tries = probes;
    size_t hash;
    ServerConnectionsPtr srv;

    do {
      HT_DEBUGF("get_connection to Service: %s, tries=%d", m_srv_name.c_str(), tries);

      for(auto endpoint : endpoints){
        size_t hash = endpoint_hash(endpoint);
        {
          std::lock_guard<std::mutex> lock(m_mutex);
          auto it = m_srv_conns->find(hash);
          if(it != m_srv_conns->end()){
            srv = (*it).second;
          } else {
            srv = std::make_shared<ServerConnections>(
              m_srv_name, endpoint, m_ioctx, m_ctx);
            m_srv_conns->insert(std::make_pair(hash, srv));
          }
        }
        
        srv->connection(con, timeout);
        if(con != nullptr)
          return con;
      }
      
      std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    } while (m_run.load() && (probes == 0 || --tries > 0));

    return con;
  }

  void preserve(ClientConPtr con){
    size_t hash = con->endpoint_remote_hash();

    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_srv_conns->find(hash);
    if(it != m_srv_conns->end())
      (*it).second->put_back(con);
  }

  void close(ClientConPtr con){
    size_t hash = con->endpoint_remote_hash();
    con->do_close();

    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_srv_conns->find(hash);
    if(it != m_srv_conns->end() && (*it).second->empty())
      m_srv_conns->erase(it);
  }

  std::string to_str(ClientConPtr con){
    std::string s("Service ");
    s.append(m_srv_name);
    s.append(" ");
    s.append(to_string(con));
    return s;
  }
  
  void stop(){
    m_run.store(false);
  }

  virtual ~SerializedClient(){
    stop();
    
    HT_INFOF("Stop Service: %s", m_srv_name.c_str());
  }

  private:
  std::mutex        m_mutex;
  std::atomic<bool> m_run;
  std::shared_ptr<ServerConnectionsMap> m_srv_conns = {};

  std::string   m_srv_name;
  uint32_t      m_port;
  AppContextPtr m_ctx;
  IOCtxPtr      m_ioctx;
};


typedef std::shared_ptr<SerializedClient> ClientPtr;

}}

#endif // swc_core_comm_SerializedClient_h