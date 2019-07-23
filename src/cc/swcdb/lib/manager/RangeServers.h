
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_manager_RangeServers_h
#define swc_lib_manager_RangeServers_h
#include <memory>

#include "swcdb/lib/db/Protocol/req/LoadRange.h"
#include "RangeServerStatus.h"


namespace SWC { namespace server { namespace Mngr {


class RangeServers : public std::enable_shared_from_this<RangeServers> {

  public:
  RangeServers(IOCtxPtr ioctx) : m_ioctx(ioctx) { }

  virtual ~RangeServers(){}

  void init(client::ClientsPtr clients) {
    m_clients = clients;
  }
  
  uint64_t rs_set_id(EndPoints endpoints, uint64_t opt_id=0){
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto it=m_rs_status.begin();it<m_rs_status.end(); it++){
      auto h = *it;
      if(has_endpoint(h->endpoints, endpoints))
        if(h->ack)
          return h->rs_id;
        else {
          m_rs_status.erase(it);
          break;
        }
    }

    uint64_t next_id=0;
    uint64_t nxt;
    bool ok;
    do {
      if(opt_id == 0) {
        nxt = ++next_id;
      } else {
        nxt = opt_id;
        opt_id = 0;
      }
      
      ok = true;
      for(auto h : m_rs_status){
        if(nxt == h->rs_id) {
          ok = false;
          break;
        };
      }
    } while(!ok);

    RangeServerStatusPtr h = std::make_shared<RangeServerStatus>(nxt, endpoints);
    m_rs_status.push_back(h);
    return h->rs_id;
  }

  bool rs_ack_id(uint64_t rs_id , EndPoints endpoints){
    std::lock_guard<std::mutex> lock(m_mutex);
    
    RangeServerStatusPtr host;
    bool ack = false;
    for(auto h : m_rs_status){
      if(has_endpoint(h->endpoints, endpoints) && rs_id == h->rs_id){
        host = h;
        h->ack = true;
        ack = true;
        break;
      }
    }

    if(ack){
      bool rs_master = false;
      for(auto h : m_rs_status){
        rs_master = h->ack && h->role == Types::RsRole::MASTER;
        if(rs_master)
          break;
      }
      if(!rs_master){
        host->role = Types::RsRole::MASTER;
        asio::post(
          *m_ioctx.get(), 
          [ptr=shared_from_this(), endpoints=host->endpoints, role=host->role](){ 
            ptr->load_master_ranges(endpoints, role);  
          });
      }
    }

    return ack;
  }

  void load_master_ranges(EndPoints endpoints, Types::RsRole role){
    
    client::ClientConPtr conn = m_clients->rs_service->get_connection(endpoints);

    Protocol::Req::LoadRangePtr req = 
      std::make_shared<Protocol::Req::LoadRange>(
        conn, role
      );
    req->run();
  }

  uint64_t rs_had_id(uint64_t rs_id , EndPoints endpoints){
    bool new_id_required = false;
    {
      std::lock_guard<std::mutex> lock(m_mutex);

      for(auto h : m_rs_status){
        if(rs_id == h->rs_id){
          if(has_endpoint(h->endpoints, endpoints))
            return 0; // zero=OK
          new_id_required = true;
          break;
        }
      }
    }
    return rs_set_id(endpoints, new_id_required ? 0 : rs_id);
  }

  void rs_shutdown(uint64_t rs_id , EndPoints endpoints){
    std::lock_guard<std::mutex> lock(m_mutex);

    bool was_master=false;
    for(auto it=m_rs_status.begin();it<m_rs_status.end(); it++){
      auto h = *it;
      if(has_endpoint(h->endpoints, endpoints)){
        m_rs_status.erase(it);
        was_master = h->role == Types::RsRole::MASTER;
        break;
      }
    }
    if(was_master && m_rs_status.size() > 0){
      auto host  = *m_rs_status.begin();
      host->role = Types::RsRole::MASTER;
      asio::post(
        *m_ioctx.get(), 
        [ptr=shared_from_this(), endpoints=host->endpoints, role=host->role](){ 
          ptr->load_master_ranges(endpoints, role);  
      });
    }

  }

  std::string to_string(){
    std::string s("RangeServers:\n");
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      for(auto h : m_rs_status)
        s.append(h->to_string());
    }
    return s;
  }


  private:
  std::mutex    m_mutex;
  IOCtxPtr      m_ioctx;
  std::vector<RangeServerStatusPtr> m_rs_status;
  client::ClientsPtr  m_clients;

};
typedef std::shared_ptr<RangeServers> RangeServersPtr;


}}}

#endif // swc_lib_manager_RangeServers_h