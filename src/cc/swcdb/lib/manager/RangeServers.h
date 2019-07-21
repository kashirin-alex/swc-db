
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_manager_RangeServers_h
#define swc_lib_manager_RangeServers_h

#include "swcdb/lib/db/Protocol/params/HostEndPoints.h"
#include <memory>

namespace SWC { namespace server { namespace Mngr {

class RangeServerStatus : public Protocol::Params::HostEndPoints {

  public:
  RangeServerStatus(uint64_t rs_id, EndPoints endpoints)
                    : ack(false), rs_id(rs_id), 
                    Protocol::Params::HostEndPoints(endpoints) {}

  virtual ~RangeServerStatus(){}

  std::string to_string(){
    std::string s(" RS-status(");

    s.append("ID=");
    s.append(std::to_string(rs_id));
    s.append(" ACK=");
    s.append(ack?"true":"false");
    s.append(" ");
    s.append(Protocol::Params::HostEndPoints::to_string());
    
    s.append(")\n");
    return s;
  }

  bool      ack;
  uint64_t  rs_id;

};
typedef std::shared_ptr<RangeServerStatus> RangeServerStatusPtr;

class RangeServers {

  public:
  RangeServers() {}
  virtual ~RangeServers(){}

  
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
    
    for(auto h : m_rs_status){
      if(has_endpoint(h->endpoints, endpoints) && rs_id == h->rs_id){
        h->ack = true;
        return true;
      }
    }
    return false;
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
    
    for(auto it=m_rs_status.begin();it<m_rs_status.end(); it++){
      auto h = *it;
      if(has_endpoint(h->endpoints, endpoints)){
          m_rs_status.erase(it);
        // cols.. (h->rs_id);
        return;
      }
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
  std::mutex  m_mutex;
  std::vector<RangeServerStatusPtr> m_rs_status;

};
typedef std::shared_ptr<RangeServers> RangeServersPtr;


}}}

#endif // swc_lib_manager_RangeServers_h