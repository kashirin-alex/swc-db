/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_manager_MngrRole_h
#define swc_manager_MngrRole_h

#include "swcdb/manager/MngrStatus.h"
#include "swcdb/manager/Protocol/Mngr/req/MngrState.h"


namespace SWC { namespace Manager {


class MngrRole final {
  
  public:
  
  MngrRole(const Comm::EndPoints& endpoints);

  ~MngrRole();

  void schedule_checkin(uint32_t t_ms = 10000);

  bool is_active(cid_t cid);

  bool is_active_role(uint8_t role);

  MngrStatus::Ptr active_mngr(cid_t cid);

  MngrStatus::Ptr active_mngr_role(uint8_t role);

  void get_states(MngrsStatus& states);
  
  Comm::EndPoint get_inchain_endpoint() const;

  void req_mngr_inchain(const Comm::client::ConnQueue::ReqBase::Ptr& req);

  void fill_states(const MngrsStatus& states, uint64_t token, 
                   const Comm::ResponseCallback::Ptr& cb);

  void update_manager_addr(uint64_t hash, const Comm::EndPoint& mngr_host);
  
  void disconnection(const Comm::EndPoint& endpoint_server, 
                     const Comm::EndPoint& endpoint_client, 
                     bool srv=false);

  void stop();

  void print(std::ostream& out);


  private:
  
  void _apply_cfg();

  void managers_checkin();

  void fill_states();

  void managers_checker(size_t next, size_t total, bool flw);

  void manager_checker(MngrStatus::Ptr host, 
                       size_t next, size_t total, bool flw, 
                       const Comm::ConnHandlerPtr& conn);
  
  void update_state(const Comm::EndPoint& endpoint, 
                    DB::Types::MngrState state);

  void update_state(const Comm::EndPoints& endpoints, 
                    DB::Types::MngrState state);
  
  MngrStatus::Ptr get_host(const Comm::EndPoints& endpoints);

  MngrStatus::Ptr get_highest_state_host(const MngrStatus::Ptr& other);
  
  bool is_group_off(const MngrStatus::Ptr& other);

  void apply_role_changes();
  
  void set_mngr_inchain(const Comm::ConnHandlerPtr& mngr);


  const Comm::EndPoints           m_local_endpoints;
  const uint64_t                  m_local_token;

  std::shared_mutex               m_mutex;
  MngrsStatus                     m_states;
  std::atomic<uint8_t>            m_checkin;
  client::Mngr::Groups::Vec       m_local_groups;
  std::atomic<uint8_t>            m_local_active_role;
  bool                            m_major_updates = false;
  std::unordered_map<uint64_t,  Comm::EndPoint> m_mngrs_client_srv;
  
  std::mutex                      m_mutex_timer;
  asio::high_resolution_timer     m_check_timer; 
  bool                            m_run = true; 
  
  Comm::client::ConnQueuePtr      m_mngr_inchain;


  const Config::Property::V_GINT32::Ptr cfg_conn_probes;
  const Config::Property::V_GINT32::Ptr cfg_conn_timeout;
  const Config::Property::V_GINT32::Ptr cfg_conn_fb_failures;
  
  const Config::Property::V_GINT32::Ptr cfg_req_timeout;
  const Config::Property::V_GINT32::Ptr cfg_delay_updated;
  const Config::Property::V_GINT32::Ptr cfg_check_interval;
  const Config::Property::V_GINT32::Ptr cfg_delay_fallback;
};



}}


#endif // swc_manager_MngrRole_h
