
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_req_RangeQueryUpdate_h
#define swcdb_db_protocol_rgr_req_RangeQueryUpdate_h



#include "swcdb/core/comm/ClientConnQueue.h"
#include "swcdb/db/Protocol/Rgr/params/RangeQueryUpdate.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {

  
class RangeQueryUpdate: public Comm::client::ConnQueue::ReqBase {
  public:
  
  typedef std::function<void(const Comm::client::ConnQueue::ReqBase::Ptr&,
                             const Params::RangeQueryUpdateRsp&)> Cb_t;

  static void 
  request(const Params::RangeQueryUpdateReq& params, 
          const DynamicBuffer::Ptr& buffer,
          const Comm::EndPoints& endpoints, const Cb_t& cb, 
          const uint32_t timeout = 10000);


  RangeQueryUpdate(const Params::RangeQueryUpdateReq& params,
                   const DynamicBuffer::Ptr& buffer, 
                   const Comm::EndPoints& endpoints, const Cb_t& cb, 
                   const uint32_t timeout);

  virtual ~RangeQueryUpdate();

  void handle_no_conn() override;

  bool run() override;

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override;

  private:

  Comm::EndPoints endpoints;
  const Cb_t      cb;
};


}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/req/RangeQueryUpdate.cc"
#endif 

#endif // swcdb_db_protocol_rgr_req_RangeQueryUpdate_h
