
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Rangers_h
#define swcdb_manager_Rangers_h

#include "swcdb/manager/Ranger.h"
#include "swcdb/manager/RangersResources.h"
#include "swcdb/manager/ColumnHealthCheck.h"


namespace SWC { namespace Manager {



class Rangers final {
  public:

  const Config::Property::V_GINT32::Ptr cfg_rgr_failures;
  const Config::Property::V_GINT32::Ptr cfg_delay_rgr_chg;
  const Config::Property::V_GINT32::Ptr cfg_chk_assign;
  const Config::Property::V_GINT32::Ptr cfg_assign_due;

  const Config::Property::V_GINT32::Ptr cfg_column_health_chk;
  const Config::Property::V_GINT32::Ptr cfg_column_health_chkers;

  Rangers(const Comm::IoContext::Ptr& app_io);

  ~Rangers();

  void stop(bool shuttingdown=true);

  bool empty();
  
  void schedule_check(uint32_t t_ms = 10000);

  void rgr_report(rgrid_t rgrid, int err,
                  const Comm::Protocol::Rgr::Params::Report::RspRes& rsp);

  Ranger::Ptr rgr_get(const rgrid_t rgrid);

  void rgr_get(const rgrid_t rgrid, Comm::EndPoints& endpoints);

  void rgr_get(const Ranger::Ptr& rgr, Comm::EndPoints& endpoints);

  void rgr_list(const rgrid_t rgrid, RangerList& rangers);

  rgrid_t rgr_set_id(const Comm::EndPoints& endpoints, rgrid_t opt_rgrid=0);

  bool rgr_ack_id(rgrid_t rgrid, const Comm::EndPoints& endpoints);

  rgrid_t rgr_had_id(rgrid_t rgrid, const Comm::EndPoints& endpoints);

  void rgr_shutdown(rgrid_t rgrid, const Comm::EndPoints& endpoints);

  
  void sync();

  void update_status(RangerList new_rgr_status, bool sync_all);


  void assign_range(const Ranger::Ptr& rgr, const Range::Ptr& range);

  void range_loaded(Ranger::Ptr rgr, Range::Ptr range, 
                    int err, bool failure=false, bool verbose=true);

  void assign_ranges();

  bool update(const DB::Schema::Ptr& schema, uint64_t req_id,
              bool ack_required);
  
  void column_delete(const cid_t cid, uint64_t req_id,
                     const std::vector<rgrid_t>& rgrids);
  
  void column_compact(int& err, const cid_t cid);


  void need_health_check(const Column::Ptr& col);

  void health_check_finished(const ColumnHealthCheck::Ptr& chk);


  void print(std::ostream& out);

  private:

  bool runs_assign(bool stop);

  void assign_ranges_run();

  void next_rgr(const Range::Ptr& range, Ranger::Ptr& rs_set);

  void health_check_columns();

  Ranger::Ptr rgr_set(const Comm::EndPoints& endpoints, rgrid_t opt_rgrid=0);

  void changes(const RangerList& hosts, bool sync_all=false);

  void _changes(const RangerList& hosts, bool sync_all=false);
  
  
  std::atomic<bool>             m_run; 

  Core::MutexAtomic             m_mutex_timer;
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

#endif // swcdb_manager_Rangers_h