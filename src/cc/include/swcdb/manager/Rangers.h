
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_manager_Rangers_h
#define swc_manager_Rangers_h

#include "swcdb/manager/Ranger.h"
#include "swcdb/manager/RangersResources.h"
#include "swcdb/manager/ColumnHealthCheck.h"


namespace SWC { namespace Manager {



class Rangers final {
  public:

  const Property::V_GINT32::Ptr cfg_rgr_failures;
  const Property::V_GINT32::Ptr cfg_delay_rgr_chg;
  const Property::V_GINT32::Ptr cfg_chk_assign;
  const Property::V_GINT32::Ptr cfg_assign_due;

  const Property::V_GINT32::Ptr cfg_column_health_chk;
  const Property::V_GINT32::Ptr cfg_column_health_chkers;

  Rangers();

  ~Rangers();

  void stop(bool shuttingdown=true);

  bool empty();
  
  void schedule_check(uint32_t t_ms = 10000);

  void rgr_report(rgrid_t rgrid, int err,
                  const Protocol::Rgr::Params::Report::RspRes& rsp);

  Ranger::Ptr rgr_get(const rgrid_t rgrid);

  void rgr_get(const rgrid_t rgrid, EndPoints& endpoints);

  void rgr_get(const Ranger::Ptr& rgr, EndPoints& endpoints);

  void rgr_list(const rgrid_t rgrid, RangerList& rangers);

  rgrid_t rgr_set_id(const EndPoints& endpoints, rgrid_t opt_rgrid=0);

  bool rgr_ack_id(rgrid_t rgrid, const EndPoints& endpoints);

  rgrid_t rgr_had_id(rgrid_t rgrid, const EndPoints& endpoints);

  void rgr_shutdown(rgrid_t rgrid, const EndPoints& endpoints);

  
  void sync();

  void update_status(RangerList new_rgr_status, bool sync_all);


  void assign_range(const Ranger::Ptr& rgr, const Range::Ptr& range);

  void range_loaded(Ranger::Ptr rgr, Range::Ptr range, 
                    int err, bool failure=false, bool verbose=true);

  void assign_ranges();

  bool update(const DB::Schema::Ptr& schema, bool ack_required);
  
  void column_delete(const cid_t cid, const std::vector<rgrid_t>& rgrids);
  
  void column_compact(int& err, const cid_t cid);


  void need_health_check(const Column::Ptr& col);

  void health_check_finished(const ColumnHealthCheck::Ptr& chk);


  std::string to_string();

  private:

  bool runs_assign(bool stop);

  void assign_ranges_run();

  void next_rgr(const EndPoints& last_rgr, Ranger::Ptr& rs_set);

  void health_check_columns();

  Ranger::Ptr rgr_set(const EndPoints& endpoints, rgrid_t opt_rgrid=0);

  void changes(const RangerList& hosts, bool sync_all=false);

  void _changes(const RangerList& hosts, bool sync_all=false);
  
  
  std::atomic<bool>             m_run; 

  std::mutex                    m_mutex_timer;
  asio::high_resolution_timer   m_timer; 

  std::mutex                    m_mutex;
  RangerList                    m_rangers;
  RangersResources              m_rangers_resources;

  std::mutex                    m_mutex_assign;
  bool                          m_runs_assign;
  int32_t                       m_assignments; 
  ColumnHealthChecks            m_columns_check;
  
};

}}

#endif // swc_manager_Rangers_h