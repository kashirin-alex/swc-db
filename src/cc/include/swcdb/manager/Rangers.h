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

  const Config::Property::Value_uint16_g::Ptr  cfg_rgr_failures;
  const Config::Property::Value_int32_g::Ptr   cfg_delay_rgr_chg;
  const Config::Property::Value_int32_g::Ptr   cfg_chk_assign;
  const Config::Property::Value_int32_g::Ptr   cfg_assign_due;

  const Config::Property::Value_int32_g::Ptr   cfg_column_health_chk;
  const Config::Property::Value_int32_g::Ptr   cfg_column_health_chkers;
  const Config::Property::Value_int32_g::Ptr   cfg_column_health_chkers_delay;

  Rangers(const Comm::IoContextPtr& app_io);

  ~Rangers() noexcept { }

  SWC_CAN_INLINE
  bool running() const noexcept {
    return m_run;
  }

  void stop(bool shuttingdown=true);

  bool empty() noexcept;

  void schedule_check(uint32_t t_ms = 10000);

  void schedule_run();

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

  void update_status(const RangerList& new_rgr_status, bool sync_all);

  void range_loaded(Ranger::Ptr rgr, Range::Ptr range, int64_t revision,
                    int err, bool failure=false, bool verbose=true);

  void assign_ranges();

  bool update(const Column::Ptr& col, const DB::Schema::Ptr& schema,
              uint64_t req_id, bool ack_required);

  void column_delete(const DB::Schema::Ptr& schema, uint64_t req_id,
                     const Core::Vector<rgrid_t>& rgrids);

  void column_compact(const Column::Ptr& col);


  void need_health_check(const Column::Ptr& col);

  void health_check_finished(const ColumnHealthCheck::Ptr& chk);

  void wait_health_check(cid_t cid=DB::Schema::NO_CID);

  void print(std::ostream& out);

  private:

  void assign_ranges_run();

  void next_rgr(const Range::Ptr& range, Ranger::Ptr& rgr_set);

  void health_check_columns();

  Ranger::Ptr rgr_set(const Comm::EndPoints& endpoints, rgrid_t opt_rgrid=0);

  void changes(const RangerList& hosts, bool sync_all=false);

  void _changes(const RangerList& hosts, bool sync_all=false);


  Core::AtomicBool              m_run;

  Core::MutexSptd               m_mutex;
  asio::high_resolution_timer   m_timer;
  RangerList                    m_rangers;
  RangersResources              m_rangers_resources;

  Core::StateRunning            m_assign;
  Core::Atomic<int32_t>         m_assignments;

  Core::MutexSptd               m_mutex_columns_check;
  ColumnHealthChecks            m_columns_check;
  int64_t                       m_columns_check_ts;

};

}}

#endif // swcdb_manager_Rangers_h