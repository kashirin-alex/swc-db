
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_manager_MngdColumns_h
#define swc_manager_MngdColumns_h

#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"

namespace SWC { namespace Manager {



class MngdColumns final {

  struct ColumnActionReq final {
    Protocol::Mngr::Params::ColumnMng params;
    std::function<void(int)>          cb;
  };
  struct ColumnFunction final {
    ColumnFunction() { }
    ColumnFunction(Protocol::Mngr::Params::ColumnMng::Function func, 
                   cid_t cid) :  func(func), cid(cid) { 
    }
    Protocol::Mngr::Params::ColumnMng::Function func;
    cid_t cid;
  };


  public:

  MngdColumns();

  ~MngdColumns();

  void stop();

  bool is_schemas_mngr(int& err);

  void active(const std::vector<cid_t>& cols);

  void is_active(int& err, cid_t cid, bool for_schema=false);

  bool has_active();

  void require_sync();

  void action(ColumnActionReq new_req);

  void update_status(Protocol::Mngr::Params::ColumnMng::Function func, 
                     DB::Schema::Ptr schema, int err, bool initial=false);

  void load_pending(cid_t cid);

  void update(Protocol::Mngr::Params::ColumnMng::Function func,
              DB::Schema::Ptr schema, int err=Error::OK);

  void remove(int &err, cid_t cid, rgrid_t rgrid);

  std::string to_string();


  private:

  bool manage(cid_t cid);

  void check_assignment();

  bool initialize();


  void columns_load();

  void columns_load_chk_ack();

  bool load_pending(cid_t cid, ColumnFunction &pending);


  cid_t get_next_cid();

  void create(int &err, DB::Schema::Ptr &schema);
  
  void update(int &err, DB::Schema::Ptr &schema, DB::Schema::Ptr old);

  void remove(int &err, cid_t cid);

  bool update(DB::Schema::Ptr schema);

  void update_status_ack(Protocol::Mngr::Params::ColumnMng::Function func,
                         DB::Schema::Ptr schema, int err);

  void actions_run();

  
  std::shared_mutex             m_mutex;
  std::atomic<bool>             m_run; 
  std::atomic<bool>             m_schemas_mngr;
  std::atomic<bool>             m_columns_set;
  std::vector<cid_t>            m_cols_active;

  std::mutex                    m_mutex_columns;
  std::queue<ColumnActionReq>   m_actions;
  std::vector<ColumnActionReq>  m_cid_pending;
  std::vector<ColumnFunction>   m_cid_pending_load;

  const Property::V_GUINT8::Ptr cfg_schema_replication;
  const Property::V_GINT32::Ptr cfg_delay_cols_init;
  
};


}} // namespace SWC::Manager

#endif // swc_manager_MngdColumns_h