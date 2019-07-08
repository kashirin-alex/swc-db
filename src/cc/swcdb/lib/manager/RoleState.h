/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_manager_RoleState_h
#define swc_app_manager_RoleState_h

#include "swcdb/lib/client/Clients.h"
#include "AppContextMngrClient.h"
#include "HostStatus.h"
#include "swcdb/lib/db/Protocol/params/MngrsState.h"

namespace SWC { namespace server { namespace Mngr {
class RoleState;
typedef std::shared_ptr<RoleState> RoleStatePtr;
}}}

#include "swcdb/lib/db/Protocol/req/MngrsState.h"


namespace SWC { namespace server { namespace Mngr {


class RoleState : public std::enable_shared_from_this<RoleState> {
  public:
  RoleState(IOCtxPtr ioctx) : m_ioctx(ioctx){}
  virtual ~RoleState() { }

  void init(EndPoints endpoints) {
    m_local_endpoints = endpoints;
    m_clients = std::make_shared<client::Clients>(
      m_ioctx, 
      std::make_shared<client::Mngr::AppContext>()
    );
    
    m_local_token = endpoints_hash(m_local_endpoints);
    asio::post(*m_ioctx.get(), [ptr=shared_from_this()]{ptr->managers_checkin();});
  }

  void apply_cfg(){
    HT_DEBUG("apply_cfg");

    client::Mngr::SelectedGroups groups = 
      m_clients->mngrs_groups->get_groups();
    
    for(auto g : groups) {
      HT_DEBUG( g->to_string().c_str());
      uint32_t pr = 0;
      for(auto endpoints : g->get_hosts()) {

        bool found = false;
        std::lock_guard<std::mutex> lock(m_mutex);
        for(auto host : m_states){
          found = has_endpoint(endpoints, host->endpoints);
          if(found)break;
        }
        if(found)continue;

        m_states.push_back(std::make_shared<HostStatus>(
          g->col_begin, g->col_end, endpoints, nullptr, ++pr)); 
      }
    }
    m_local_groups = m_clients->mngrs_groups->get_groups(m_local_endpoints);
  }
  
  bool has_endpoint(EndPoints& endpoints, EndPoints& endpoints_in){
    for(auto & e1 : endpoints){
      if(has_endpoint(e1, endpoints_in)) return true;
    }
    return false;
  }

  bool has_endpoint(EndPoint& e1, EndPoints& endpoints){
    return std::find_if(endpoints.begin(), endpoints.end(),  
            [e1](const EndPoint& e2){
              return e1.address() == e2.address() && e1.port() == e2.port();}) 
          != endpoints.end();
  }

  void timer_managers_checkin(uint32_t t_ms = 10000) {
    if(m_check_timer != nullptr) // adjust timer
      m_check_timer->cancel();

    m_check_timer = std::make_shared<asio::high_resolution_timer>(
      *m_ioctx.get(), std::chrono::milliseconds(t_ms)); 

    m_check_timer->async_wait(
      [ptr=shared_from_this()](const asio::error_code ec) {
        if (ec != asio::error::operation_aborted){
          ptr->managers_checkin();
          ptr->m_check_timer=nullptr;
        }
    }); 
    std::cout << "timer=" << t_ms << "\n";
  }

  void managers_checkin(){
    HT_DEBUG("managers_checkin");

    if(m_check_timer != nullptr) m_check_timer->cancel();

    apply_cfg();
    
    HostStatuses new_states;
    {
    std::lock_guard<std::mutex> lock(m_mutex);

    size_t total = m_states.size();
    if(total == 0) {
      timer_managers_checkin();
      return;
    }

    // request to a one manager followed local manager, incl. last's is first
    bool flw = false;
    HostStatusPtr host_chk = nullptr;

    auto it = m_states.begin();
    for(;;){

      host_chk = *it;
      
      if(it+1 == m_states.end())
        it = m_states.begin();
      else
        it++;

      if(has_endpoint(host_chk->endpoints, m_local_endpoints) && total > 1){
        if(flw) {
          timer_managers_checkin(10000);
          return;
        }
        flw = true;
        continue;
      }
      if(!flw)
        continue;
        
      if(host_chk->conn == nullptr || !host_chk->conn->is_open()){
        m_mngr_inchain = nullptr;
        host_chk->conn = m_clients->mngr_service->get_connection(
          host_chk->endpoints, std::chrono::milliseconds(conn_timeout), conn_probes);
        
        if(host_chk->conn == nullptr){
          total--;
          host_chk->state = State::OFF;
          continue;
        } else 
          total = m_states.size();

        host_chk->conn->accept_requests();
      }
      m_mngr_inchain = host_chk->conn;

      break;
    }

    new_states.assign(m_states.begin(), m_states.end());
    }
    
    fill_states(new_states);
  }
  
  void update_state(EndPoint endpoint, State state){
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto host : m_states){
      if(has_endpoint(endpoint, host->endpoints)){
        host->state = state;
      }
    }
  }

  void update_state(EndPoints endpoints, State state){
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto host : m_states){
      if(has_endpoint(endpoints, host->endpoints)){
        host->state = state;
      }
    }
  }
  
  HostStatusPtr get_host(EndPoints endpoints){
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto host : m_states){
      if(has_endpoint(endpoints, host->endpoints))
        return host;
    }
    return nullptr;
  }

  bool has_state(uint64_t begin, uint64_t end, State state){
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for(auto host : m_states){
      if(host->col_begin == begin 
        && host->col_end == end && host->state == state)
        return true;
    }
    return false;
  }

  HostStatusPtr get_highest_state_host(uint64_t begin, uint64_t end){
    std::lock_guard<std::mutex> lock(m_mutex);

    HostStatusPtr h = nullptr;
    for(auto host : m_states){
      if(host->col_begin == begin && host->col_end == end 
        && (h == nullptr || h->state < host->state)){
        h = host;
      }
    }
    return h;
  }
  
  bool is_off(uint64_t begin, uint64_t end){
    std::lock_guard<std::mutex> lock(m_mutex);
    
    bool offline = true;
    for(auto host : m_states){
      if(host->col_begin == begin 
        && host->col_end == end && host->state != State::OFF)
        offline = false;
    }
    return offline;
  }

  void fill_states(HostStatuses states, 
                   uint64_t token=0, ResponseCallbackPtr cb=nullptr){

    bool turn_around = token == m_local_token;
    bool local_updated = false;

    std::cout << "BEFORE:\n";
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      for(auto h : m_states)
        std::cout << h->to_string() << "\n";
    } 
    for(auto host : states){

      bool local = has_endpoint(host->endpoints, m_local_endpoints);

      if(local && token == 0 && host->state < State::STANDBY){
        update_state(host->endpoints, State::STANDBY);

      } else if(host->state >= State::STANDBY){
        HostStatusPtr high_set = 
          get_highest_state_host(host->col_begin, host->col_end);

        if(host->priority == high_set->priority 
          && high_set->state == State::ACTIVE)
          continue;

        else if(high_set->state == State::ACTIVE)
          update_state(host->endpoints, State::STANDBY);

        else if(host->priority > high_set->priority){
          if(high_set->state == State::STANDBY)
            continue;
          update_state(host->endpoints, (State)(high_set->state-1));

        } else
          update_state(host->endpoints, (State)(high_set->state+1));

      } else {
        HostStatusPtr host_set = get_host(host->endpoints);
        if(host->state == host_set->state) 
          continue;

        if(host->state == State::OFF)
          update_state(host->endpoints, State::OFF);
        else
          update_state(host->endpoints, 
            host->state> host_set->state? host->state: host_set->state);
      }

      if(local) local_updated = true;
    }
    
    for(auto host : states){
      if(!is_off(host->col_begin, host->col_end))
        continue;
      auto hosts_pr_group = 
        m_clients->mngrs_groups->get_endpoints(
          host->col_begin, host->col_end);
      for(auto h : hosts_pr_group){
        if(has_endpoint(h, host->endpoints) 
          || !has_endpoint(h, m_local_endpoints))
          continue;
            
        HostStatusPtr l_host = get_host(m_local_endpoints);
        HostStatusPtr l_hight = get_highest_state_host(l_host->col_begin, l_host->col_end);
        if(l_hight->state < State::WANT) {
          update_state(m_local_endpoints, State::WANT);
          local_updated = true;
        }
        break;
      }
    }

    std::cout << "AFTER:\n";
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      for(auto h : m_states)
        std::cout << h->to_string() << "\n";
    } 
    
    if(token == 0 || !turn_around) {
      if(token == 0)
        token = m_local_token;

      HostStatuses updated_states;
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        updated_states.assign(m_states.begin(), m_states.end());
      }
      
      Protocol::Req::MngrsStatePtr req = 
        std::make_shared<Protocol::Req::MngrsState>(
          m_mngr_inchain, updated_states, token, cb, shared_from_this());

      if(!req->run((conn_probes*conn_timeout+req_timeout)*updated_states.size()))
        timer_managers_checkin(3000);
      return;
    }
    
    
    std::cout << " token=" << token << " turn_around="<<turn_around<< "\n";
    if(turn_around){

      if(cb != nullptr)
        cb->response_ok();

      if(local_updated)
        timer_managers_checkin(50);
      else
        timer_managers_checkin(30000);
    }
  }

  bool is_active(size_t cid){
    return false;
  }

  bool is_active(size_t begin, size_t end, EndPoint endpoint){
    
    return false;
    /*
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto host : m_states){
      if(std::find_if(host->endpoints.begin(), host->endpoints.end(), 
            [endpoint](const EndPoint & e2){
            return endpoint.address() == e2.address() 
                    && endpoint.port() == e2.port();}) 
        != host->endpoints.end()
        && (host->active == true 
        && host->conn != nullptr 
        && host->conn->is_open())
        && host->col_begin <= begin 
        && (host->col_end == 0 || host->col_end >= end)){
          std::cout << "RoleState is_active, " 
                    << endpoint.address() <<":" <<  endpoint.port() << "\n";
          return true;
      }
    }

    std::cout << "RoleState not-is_active, " 
              << endpoint.address() <<":" <<  endpoint.port() << "\n";
    return false;
     */
  }

  private:
  
  IOCtxPtr                     m_ioctx;
  HostStatuses                 m_states;
  std::mutex                   m_mutex;
  TimerPtr                     m_check_timer; 
  
  client::ClientsPtr           m_clients;
  client::ClientConPtr         m_mngr_inchain = nullptr;
  EndPoints                    m_local_endpoints;
  uint64_t                     m_local_token;
  client::Mngr::SelectedGroups m_local_groups;

  int       conn_probes = 3;
  uint32_t  conn_timeout = 1000;
  uint32_t  req_timeout = 30000;
};

}}}


namespace SWC {
namespace Protocol {
namespace Req {
  
void MngrsState::disconnected() {
  //role_state->update_state(
  //  conn->endpoint_remote, server::Mngr::State::NOTSET);
  role_state->timer_managers_checkin(10000);
}

}}}

#endif // swc_app_manager_RoleState_h