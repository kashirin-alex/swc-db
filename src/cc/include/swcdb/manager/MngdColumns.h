/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_MngdColumns_h
#define swcdb_manager_MngdColumns_h

#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"
#include <forward_list>

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
    ~ColumnReq() noexcept { }
  };


  const Config::Property::Value_uint8_g::Ptr   cfg_schema_replication;
  const Config::Property::Value_uint64_g::Ptr  cfg_schemas_store_cap;
  const Config::Property::Value_int32_g::Ptr   cfg_schemas_store_blksz;
  const Config::Property::Value_enum_g::Ptr    cfg_schemas_store_encoder;
  const Config::Property::Value_int32_g::Ptr   cfg_delay_cols_init;


  MngdColumns();

  MngdColumns(const MngdColumns&) = delete;
  MngdColumns(MngdColumns&&) = delete;
  MngdColumns& operator=(const MngdColumns&) = delete;
  MngdColumns& operator=(MngdColumns&&) = delete;

  ~MngdColumns() noexcept { }

  SWC_CAN_INLINE
  bool running() const noexcept {
    return m_run;
  }

  void stop();

  void create_schemas_store();

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

  void set_expect(cid_t cid_begin, cid_t cid_end, uint64_t total,
                  cids_t&& columns, bool initial);

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


  Core::AtomicBool          m_run;

  Core::StateRunning        m_columns_load;

  Core::MutexSptd           m_mutex_schemas;
  Core::AtomicBool          m_schemas_set;

  Core::MutexSptd           m_mutex_active;
  bool                      m_cid_active;
  cid_t                     m_cid_begin;
  cid_t                     m_cid_end;
  uint64_t                  m_expected_remain;
  Core::Atomic<cid_t>       m_last_used_cid;
  std::forward_list<cid_t>  m_expected_load;

  Core::MutexSptd                              m_mutex_actions;
  Core::QueueSafe<ColumnReq::Ptr>              m_actions;
  std::unordered_map<uint64_t, ColumnReq::Ptr> m_actions_pending;

  constexpr static uint64_t STATE_COLUMNS_NOT_INITIALIZED = UINT64_MAX;
};


}} // namespace SWC::Manager

#endif // swcdb_manager_MngdColumns_h