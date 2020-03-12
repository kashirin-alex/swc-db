
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_lib_db_protocol_mngr_req_ColumnMng_h
#define swc_lib_db_protocol_mngr_req_ColumnMng_h


#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"
#include "swcdb/core/comm/ClientConnQueue.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

  
class ColumnMng: public client::ConnQueue::ReqBase {
  public:

  using Func = Params::ColumnMng::Function;
  typedef std::function<void(client::ConnQueue::ReqBase::Ptr, int)> Cb_t;

  static void create(DB::Schema::Ptr schema, const Cb_t cb, 
                     const uint32_t timeout = 10000);

  static void modify(DB::Schema::Ptr schema, const Cb_t cb, 
                     const uint32_t timeout = 10000);

  static void remove(DB::Schema::Ptr schema, const Cb_t cb, 
                     const uint32_t timeout = 10000);

  static void request(Func func, DB::Schema::Ptr schema, const Cb_t cb, 
                      const uint32_t timeout = 10000);


  ColumnMng(const Params::ColumnMng& params, const Cb_t cb, 
            const uint32_t timeout);

  virtual ~ColumnMng();

  void handle_no_conn() override;

  bool run(uint32_t timeout=0) override;

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override;

  private:
  
  void clear_endpoints();

  const Cb_t  cb;
  EndPoints   endpoints;
};


}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/ColumnMng.cc"
#endif 

#endif // swc_lib_db_protocol_mngr_req_ColumnMng_h
