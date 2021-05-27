/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_req_ColumnGet_h
#define swcdb_db_protocol_bkr_req_ColumnGet_h


#include "swcdb/db/Protocol/Bkr/req/ColumnGet_Base.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


class ColumnGet: public ColumnGet_Base {
  public:

  using Flag = Mngr::Params::ColumnGetReq::Flag;
  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&,
                             int, const Mngr::Params::ColumnGetRsp&)> Cb_t;


  static void schema(const SWC::client::Clients::Ptr& clients,
                     const std::string& name, Cb_t&& cb,
                     const uint32_t timeout = 10000);

  static void schema(const SWC::client::Clients::Ptr& clients,
                     cid_t cid, Cb_t&& cb,
                     const uint32_t timeout = 10000);

  static void cid(const SWC::client::Clients::Ptr& clients,
                  const std::string& name, Cb_t&& cb,
                  const uint32_t timeout = 10000);

  static void request(const SWC::client::Clients::Ptr& clients,
                      Flag flag, const std::string& name, Cb_t&& cb,
                      const uint32_t timeout = 10000);

  static void request(const SWC::client::Clients::Ptr& clients,
                      Flag flag, cid_t cid, Cb_t&& cb,
                      const uint32_t timeout = 10000);


  ColumnGet(const SWC::client::Clients::Ptr& clients,
            const Mngr::Params::ColumnGetReq& params, Cb_t&& cb,
            const uint32_t timeout);

  virtual ~ColumnGet() { }

  protected:
  virtual void callback(int error,
                        const Mngr::Params::ColumnGetRsp& rsp)  override;

  private:
  const Cb_t                      cb;

};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Bkr/req/ColumnGet.cc"
#endif

#endif // swcdb_db_protocol_bkr_req_ColumnGet_h
