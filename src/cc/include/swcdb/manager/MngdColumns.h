
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_manager_MngdColumns_h
#define swc_manager_MngdColumns_h

#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"

namespace SWC { namespace Manager {



class MngdColumns final {

  struct ColumnFunction final {
    ColumnFunction() { }
    ColumnFunction(Protocol::Mngr::Params::ColumnMng::Function func, 
                   cid_t cid) :  func(func), cid(cid) { 
    }
    Protocol::Mngr::Params::ColumnMng::Function func;
    cid_t cid;
  };

  public:

  struct ColumnReq final : public Protocol::Mngr::Params::ColumnMng,
                           public ResponseCallback {
    typedef std::shared_ptr<ColumnReq> Ptr;
    ColumnReq(const ConnHandlerPtr& conn, const Event::Ptr& ev)
              : ResponseCallback(conn, ev) { }
  };

  MngdColumns();

  ~MngdColumns();

  void stop();

  void reset(bool schemas_mngr);


  bool is_schemas_mngr(int& err);

  bool has_active();

  bool is_active(cid_t cid);

  void is_active(int& err, cid_t cid, bool for_schema=false);

  void change_active(const cid_t cid_begin, const cid_t cid_end, 
                     bool has_cols);


  void require_sync();

  void action(const ColumnReq::Ptr& new_req);

  void update_status(Protocol::Mngr::Params::ColumnMng::Function func, 
                     DB::Schema::Ptr& schema, int err, bool initial=false);

  void load_pending(cid_t cid);

  void update(Protocol::Mngr::Params::ColumnMng::Function func,
              const DB::Schema::Ptr& schema, int err=Error::OK);

  void remove(int &err, cid_t cid, rgrid_t rgrid);

  std::string to_string();


  bool initialize();

  void columns_load_chk_ack();

  private:

  void check_assignment();


  void columns_load();

  bool load_pending(cid_t cid, ColumnFunction& pending);


  cid_t get_next_cid();

  void create(int &err, DB::Schema::Ptr& schema);
  
  void update(int &err, DB::Schema::Ptr& schema, const DB::Schema::Ptr& old);

  void remove(int &err, cid_t cid);

  bool update(DB::Schema::Ptr& schema);

  void update_status_ack(Protocol::Mngr::Params::ColumnMng::Function func,
                         const DB::Schema::Ptr& schema, int err);

  void actions_run();

  
  std::shared_mutex             m_mutex;
  std::atomic<bool>             m_run; 
  std::atomic<bool>             m_schemas_set;
  bool                          m_cid_active;
  cid_t                         m_cid_begin;
  cid_t                         m_cid_end;

  std::mutex                    m_mutex_columns;
  QueueSafe<ColumnReq::Ptr>     m_actions;
  std::vector<ColumnReq::Ptr>   m_cid_pending;
  std::vector<ColumnFunction>   m_cid_pending_load;

  const Property::V_GUINT8::Ptr cfg_schema_replication;
  const Property::V_GINT32::Ptr cfg_delay_cols_init;
  
};


}} // namespace SWC::Manager

#endif // swc_manager_MngdColumns_h