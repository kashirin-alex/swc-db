
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_manager_Rangers_h
#define swc_manager_Rangers_h

#include "swcdb/manager/Ranger.h"


namespace SWC { namespace Manager {



class Rangers final {
  public:

  Rangers();

  ~Rangers();

  void stop(bool shuttingdown=true);

  void schedule_assignment_check(uint32_t t_ms = 10000);


  void rgr_get(const uint64_t id, EndPoints& endpoints);

  void rgr_list(const uint64_t rgr_id, RangerList& rangers);

  uint64_t rgr_set_id(const EndPoints& endpoints, uint64_t opt_id=0);

  bool rgr_ack_id(uint64_t id, const EndPoints& endpoints);

  uint64_t rgr_had_id(uint64_t id, const EndPoints& endpoints);

  void rgr_shutdown(uint64_t id, const EndPoints& endpoints);

  
  void sync();

  void update_status(RangerList new_rgr_status, bool sync_all);


  void assign_range_chk_last(int err, Ranger::Ptr rs_chk);

  void assign_range(Ranger::Ptr rgr, Range::Ptr range);

  void range_loaded(Ranger::Ptr rgr, Range::Ptr range, 
                    int err, bool failure=false, bool verbose=true);

  void assign_ranges();

  bool update(DB::Schema::Ptr schema, bool ack_required);
  
  void column_delete(const int64_t cid, const std::vector<uint64_t>& rgr_ids);
  
  void column_compact(int& err, const int64_t cid);

  std::string to_string();

  private:

  const bool runs_assign(bool stop);

  void assign_ranges_run();

  void next_rgr(Files::RgrData::Ptr &last_rgr, Ranger::Ptr &rs_set);

  void assign_range(Ranger::Ptr rgr, Range::Ptr range, 
                    Files::RgrData::Ptr last_rgr);

  Ranger::Ptr rgr_set(const EndPoints& endpoints, uint64_t opt_id=0);

  void changes(RangerList& hosts, bool sync_all=false);
  
  std::atomic<bool>             m_run; 

  std::mutex                    m_mutex_timer;
  asio::high_resolution_timer   m_assign_timer; 

  std::mutex                    m_mutex;
  RangerList                    m_rangers;

  std::mutex                    m_mutex_assign;
  bool                          m_runs_assign;
  std::atomic<int>              m_assignments; 
  
  const Property::V_GINT32::Ptr cfg_rgr_failures;
  const Property::V_GINT32::Ptr cfg_delay_rgr_chg;
  const Property::V_GINT32::Ptr cfg_chk_assign;
  const Property::V_GINT32::Ptr cfg_assign_due;
  
};

}}

#endif // swc_manager_Rangers_h