
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_manager_MngdColumns_h
#define swc_manager_MngdColumns_h

#include "swcdb/manager/db/Schema.h"

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
                   int64_t cid) :  func(func), cid(cid) { 
    }
    Protocol::Mngr::Params::ColumnMng::Function func;
    int64_t cid;
  };


  public:

  MngdColumns();

  ~MngdColumns();

  void stop();

  const bool is_root_mngr();

  void active(const std::vector<int64_t>& cols);

  void is_active(int& err, int64_t cid, bool for_schema=false);

  bool has_active();

  void require_sync();

  void action(ColumnActionReq new_req);

  void update_status(Protocol::Mngr::Params::ColumnMng::Function func, 
                     DB::Schema::Ptr schema, int err, bool initial=false);

  void load_pending(int64_t cid);

  void update(Protocol::Mngr::Params::ColumnMng::Function func,
              DB::Schema::Ptr schema, int err=Error::OK);

  void remove(int &err, int64_t cid, int64_t rgr_id);

  std::string to_string();


  private:

  bool manage(int64_t cid);

  void check_assignment();

  bool initialize();


  void columns_load();

  void columns_load_chk_ack();

  bool load_pending(int64_t cid, ColumnFunction &pending);


  int64_t get_next_cid();

  void create(int &err, DB::Schema::Ptr &schema);
  
  void update(int &err, DB::Schema::Ptr &schema, DB::Schema::Ptr old);

  void remove(int &err, int64_t cid);

  bool update(DB::Schema::Ptr schema);

  void update_status_ack(Protocol::Mngr::Params::ColumnMng::Function func,
                         DB::Schema::Ptr schema, int err);

  void actions_run();

  
  std::shared_mutex             m_mutex;
  std::atomic<bool>             m_run; 
  std::atomic<bool>             m_root_mngr;
  std::atomic<bool>             m_columns_set;
  std::vector<int64_t>          m_cols_active;

  std::mutex                    m_mutex_columns;
  std::queue<ColumnActionReq>   m_actions;
  std::vector<ColumnActionReq>  m_cid_pending;
  std::vector<ColumnFunction>   m_cid_pending_load;

  const Property::V_GUINT8::Ptr  cfg_schema_replication;
  const Property::V_GINT32::Ptr cfg_delay_cols_init;
  
};


}} // namespace SWC::Manager

#endif // swc_manager_MngdColumns_h