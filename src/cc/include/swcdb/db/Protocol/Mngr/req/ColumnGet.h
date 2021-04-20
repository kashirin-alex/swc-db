/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_req_ColumnGet_h
#define swcdb_db_protocol_mngr_req_ColumnGet_h


#include "swcdb/db/Protocol/Mngr/params/ColumnGet.h"
#include "swcdb/core/comm/ClientConnQueue.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class ColumnGet: public client::ConnQueue::ReqBase {
  public:

  using Flag = Params::ColumnGetReq::Flag;
  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&,
                             int, const Params::ColumnGetRsp&)> Cb_t;


  static void schema(const std::string& name, Cb_t&& cb,
                     const uint32_t timeout = 10000);

  static void schema(cid_t cid, Cb_t&& cb,
                     const uint32_t timeout = 10000);

  static void cid(const std::string& name, Cb_t&& cb,
                  const uint32_t timeout = 10000);

  static void request(Flag flag, const std::string& name, Cb_t&& cb,
                      const uint32_t timeout = 10000);

  static void request(Flag flag, cid_t cid, Cb_t&& cb,
                      const uint32_t timeout = 10000);


  ColumnGet(const Params::ColumnGetReq& params, Cb_t&& cb,
            const uint32_t timeout);

  virtual ~ColumnGet() { }

  void handle_no_conn() override;

  bool run() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  private:

  void clear_endpoints();

  const Cb_t  cb;
  EndPoints   endpoints;
};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/ColumnGet.cc"
#endif

#endif // swcdb_db_protocol_mngr_req_ColumnGet_h
