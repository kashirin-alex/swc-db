
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_MngdColumns_h
#define swcdb_manager_MngdColumns_h

#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"

namespace SWC { namespace Manager {



class MngdColumns final {

  struct ColumnFunction final {
    ColumnFunction() { }
    ColumnFunction(Comm::Protocol::Mngr::Params::ColumnMng::Function func, 
                   cid_t cid) :  func(func), cid(cid) { 
    }
    Comm::Protocol::Mngr::Params::ColumnMng::Function func;
    cid_t cid;
  };

  public:

  struct ColumnReq final : public Comm::Protocol::Mngr::Params::ColumnMng,
                           public Comm::ResponseCallback {
    typedef std::shared_ptr<ColumnReq> Ptr;
    ColumnReq(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev)
              : Comm::ResponseCallback(conn, ev) { }
  };

  MngdColumns();

  ~MngdColumns();

  void stop();

  void reset(bool schemas_mngr);


  bool is_schemas_mngr(int& err);

  bool has_active();

  bool has_cid_pending_load();
  
  bool is_active(cid_t cid);

  bool active(cid_t& cid_begin, cid_t& cid_end);

  Column::Ptr get_column(int& err, cid_t cid);

  void change_active(const cid_t cid_begin, const cid_t cid_end, 
                     bool has_cols);


  void require_sync();

  void action(const ColumnReq::Ptr& new_req);

  void set_expect(const std::vector<cid_t>& columns, bool initial);

  void update_status(Comm::Protocol::Mngr::Params::ColumnMng::Function func, 
                     DB::Schema::Ptr& schema, int err, bool initial=false);

  void load_pending(cid_t cid);

  void update(Comm::Protocol::Mngr::Params::ColumnMng::Function func,
              const DB::Schema::Ptr& schema, int err=Error::OK);

  void remove(int &err, cid_t cid, rgrid_t rgrid);

  void print(std::ostream& out);


  bool initialize();

  void columns_load_chk_ack();

  private:

  void check_assignment();


  bool columns_load();

  bool load_pending(cid_t cid, ColumnFunction& pending);


  cid_t get_next_cid();

  void create(int &err, DB::Schema::Ptr& schema);
  
  void update(int &err, DB::Schema::Ptr& schema, const DB::Schema::Ptr& old);

  void remove(int &err, cid_t cid);

  bool update(DB::Schema::Ptr& schema);

  void update_status_ack(
      Comm::Protocol::Mngr::Params::ColumnMng::Function func,
      const DB::Schema::Ptr& schema, int err);

  void actions_run();

  
  std::shared_mutex                   m_mutex;
  std::atomic<bool>                   m_run; 
  std::atomic<bool>                   m_schemas_set;
  std::atomic<bool>                   m_cid_active;
  cid_t                               m_cid_begin;
  cid_t                               m_cid_end;

  std::mutex                          m_mutex_columns;
  std::atomic<bool>                   m_expected_ready;
  std::vector<cid_t>                  m_expected_load;

  Core::QueueSafe<ColumnReq::Ptr>     m_actions;
  std::vector<ColumnReq::Ptr>         m_cid_pending;
  std::vector<ColumnFunction>         m_cid_pending_load;

  const Config::Property::V_GUINT8::Ptr cfg_schema_replication;
  const Config::Property::V_GINT32::Ptr cfg_delay_cols_init;
  
};


}} // namespace SWC::Manager

#endif // swcdb_manager_MngdColumns_h