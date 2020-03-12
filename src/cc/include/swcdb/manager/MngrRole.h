/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_manager_MngrRole_h
#define swc_manager_MngrRole_h

#include "swcdb/manager/MngrStatus.h"
#include "swcdb/manager/Protocol/Mngr/req/MngrState.h"


namespace SWC { namespace Manager {


class MngrRole final {
  
  public:
  
  MngrRole(const EndPoints& endpoints);

  ~MngrRole();

  void schedule_checkin(uint32_t t_ms = 10000);

  bool is_active(size_t cid);

  MngrStatus::Ptr active_mngr(size_t begin, size_t end);

  void req_mngr_inchain(client::ConnQueue::ReqBase::Ptr req);

  void fill_states(MngrsStatus states, uint64_t token, 
                   ResponseCallback::Ptr cb);

  void update_manager_addr(uint64_t hash, const EndPoint& mngr_host);
  
  bool disconnection(const EndPoint& endpoint_server, const EndPoint& endpoint_client, 
                     bool srv=false);
  
  bool require_sync();

  void stop();

  const std::string to_string();


  private:
  
  void apply_cfg();

  void managers_checkin();

  void fill_states();

  void managers_checker(int next, size_t total, bool flw);

  void manager_checker(MngrStatus::Ptr host, int next, size_t total, bool flw,
                       ConnHandlerPtr conn);
  
  void update_state(EndPoint endpoint, Types::MngrState state);

  void update_state(const EndPoints& endpoints, Types::MngrState state);
  
  MngrStatus::Ptr get_host(const EndPoints& endpoints);

  MngrStatus::Ptr get_highest_state_host(uint64_t begin, uint64_t end);
  
  bool is_off(uint64_t begin, uint64_t end);

  void set_active_columns();
  
  void set_mngr_inchain(ConnHandlerPtr mngr);


  const EndPoints                 m_local_endpoints;
  const uint64_t                  m_local_token;

  std::shared_mutex               m_mutex;
  MngrsStatus                     m_states;
  std::atomic<uint8_t>            m_checkin;
  client::Mngr::Groups::Selected  m_local_groups;
  bool                            m_major_updates = false;
  std::unordered_map<uint64_t,  EndPoint> m_mngrs_client_srv;
  
  std::mutex                      m_mutex_timer;
  asio::high_resolution_timer     m_check_timer; 
  bool                            m_run = true; 
  
  client::ConnQueue::Ptr          m_mngr_inchain;


  const Property::V_GINT32::Ptr cfg_conn_probes;
  const Property::V_GINT32::Ptr cfg_conn_timeout;
  const Property::V_GINT32::Ptr cfg_conn_fb_failures;
  
  const Property::V_GINT32::Ptr cfg_req_timeout;
  const Property::V_GINT32::Ptr cfg_delay_updated;
  const Property::V_GINT32::Ptr cfg_check_interval;
  const Property::V_GINT32::Ptr cfg_delay_fallback;
};



}}
#endif // swc_manager_MngrRole_h