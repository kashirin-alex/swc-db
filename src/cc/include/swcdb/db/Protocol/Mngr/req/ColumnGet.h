
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_protocol_mngr_req_ColumnGet_h
#define swc_db_protocol_mngr_req_ColumnGet_h


#include "swcdb/db/Protocol/Mngr/params/ColumnGet.h"
#include "swcdb/core/comm/ClientConnQueue.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

  
class ColumnGet: public client::ConnQueue::ReqBase {
  public:
  
  using Flag = Params::ColumnGetReq::Flag;
  typedef std::function<void(client::ConnQueue::ReqBase::Ptr, 
                             int, const Params::ColumnGetRsp&)> Cb_t;


  static void schema(const std::string& name, const Cb_t cb, 
                     const uint32_t timeout = 10000);
  
  static void schema(cid_t cid, const Cb_t cb, 
                     const uint32_t timeout = 10000);

  static void cid(const std::string& name, const Cb_t cb, 
                  const uint32_t timeout = 10000);

  static void request(Flag flag, const std::string& name, const Cb_t cb, 
                      const uint32_t timeout = 10000);

  static void request(Flag flag, cid_t cid, const Cb_t cb, 
                      const uint32_t timeout = 10000);


  ColumnGet(const Params::ColumnGetReq& params, const Cb_t cb, 
            const uint32_t timeout);

  virtual ~ColumnGet();

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
#include "swcdb/db/Protocol/Mngr/req/ColumnGet.cc"
#endif 

#endif // swc_db_protocol_mngr_req_ColumnGet_h
