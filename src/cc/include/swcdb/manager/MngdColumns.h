/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_MngdColumns_h
#define swcdb_manager_MngdColumns_h

#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"

namespace SWC { namespace Manager {


class MngdColumns final {
  public:

  struct ColumnReq final : public Comm::Protocol::Mngr::Params::ColumnMng,
                           public Comm::ResponseCallback {
    typedef std::shared_ptr<ColumnReq> Ptr;
    uint64_t id;
    SWC_CAN_INLINE
    ColumnReq(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev)
              noexcept : Comm::ResponseCallback(conn, ev), id(0) {
    }
  };

  MngdColumns();

  //~MngdColumns() { }

  void stop();

  void reset(bool schemas_mngr);


  bool is_schemas_mngr(int& err);

  bool has_active() noexcept;

  bool is_active(cid_t cid) noexcept;

  bool active(cid_t& cid_begin, cid_t& cid_end) noexcept;

  bool expected_ready() noexcept;

  void columns_ready(int& err);

  Column::Ptr get_column(int& err, cid_t cid);

  void change_active(const cid_t cid_begin, const cid_t cid_end,
                     bool has_cols);


  void require_sync();

  void action(const ColumnReq::Ptr& req);

  void set_expect(cid_t cid_begin, cid_t cid_end,
                  Core::Vector<cid_t>&& columns, bool initial);

  void update_status(Comm::Protocol::Mngr::Params::ColumnMng::Function func,
                     const DB::Schema::Ptr& schema, int err, uint64_t req_id,
                     bool initial=false);

  void update(Comm::Protocol::Mngr::Params::ColumnMng::Function func,
              const DB::Schema::Ptr& schema, int err, uint64_t req_id);

  void remove(const DB::Schema::Ptr& schema, rgrid_t rgrid, uint64_t req_id);

  void print(std::ostream& out);


  bool initialize();

  private:

  bool columns_load();

  cid_t get_next_cid();

  void create(int &err, DB::Schema::Ptr& schema);

  void update(int &err, DB::Schema::Ptr& schema, const DB::Schema::Ptr& old);

  void update_status_ack(
      Comm::Protocol::Mngr::Params::ColumnMng::Function func,
      const DB::Schema::Ptr& schema, int err, uint64_t req_id);

  void run_actions();


  Core::AtomicBool       m_run;

  Core::StateRunning     m_columns_load;

  Core::MutexSptd        m_mutex_schemas;
  Core::AtomicBool       m_schemas_set;

  Core::MutexSptd        m_mutex_active;
  bool                   m_cid_active;
  cid_t                  m_cid_begin;
  cid_t                  m_cid_end;

  Core::MutexSptd        m_mutex_expect;
  bool                   m_expected_ready;
  Core::Vector<cid_t>    m_expected_load;

  Core::MutexSptd                              m_mutex_actions;
  Core::QueueSafe<ColumnReq::Ptr>              m_actions;
  std::unordered_map<uint64_t, ColumnReq::Ptr> m_actions_pending;

  const Config::Property::V_GUINT8::Ptr cfg_schema_replication;
  const Config::Property::V_GINT32::Ptr cfg_delay_cols_init;

};


}} // namespace SWC::Manager

#endif // swcdb_manager_MngdColumns_h