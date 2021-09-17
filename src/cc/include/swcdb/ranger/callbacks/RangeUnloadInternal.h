/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_RangeUnloadInternal_h
#define swcdb_ranger_callbacks_RangeUnloadInternal_h

#include "swcdb/db/Protocol/Mngr/req/RangeUnloaded.h"


namespace SWC { namespace Ranger { namespace Callback {


class RangeUnloadInternal final : public ManageBase {
  public:

  typedef std::shared_ptr<RangeUnloadInternal>  Ptr;
  const cid_t                                   cid;
  const rid_t                                   rid;

  SWC_CAN_INLINE
  RangeUnloadInternal(const cid_t a_cid, const rid_t a_rid) noexcept
        : ManageBase(nullptr, nullptr, ManageBase::RANGE_UNLOAD_INTERNAL),
          cid(a_cid), rid(a_rid) {
  }

  virtual ~RangeUnloadInternal() noexcept { }

  void run() override { }

  void response(int&) override { }

  void send_error(int, const std::string&) override { }

  void response_ok() override { }

  void response() {
    Comm::Protocol::Mngr::Req::RangeUnloaded<ReqData>::request(
      Comm::Protocol::Mngr::Params::RangeUnloadedReq(cid, rid),
      10000, cid, rid
    );
  }

  private:

  struct ReqData {
    const cid_t cid;
    const rid_t rid;
    SWC_CAN_INLINE
    ReqData(cid_t a_cid, rid_t a_rid) noexcept : cid(a_cid), rid(a_rid) { }
    SWC_CAN_INLINE
    cid_t get_cid() const noexcept {
      return cid;
    }
    SWC_CAN_INLINE
    client::Clients::Ptr& get_clients() noexcept {
      return Env::Clients::get();
    }
    SWC_CAN_INLINE
    bool valid() noexcept {
      return !Env::Rgr::is_not_accepting();
    }
    SWC_CAN_INLINE
    void callback(
        const Comm::client::ConnQueue::ReqBase::Ptr& req,
        const Comm::Protocol::Mngr::Params::RangeUnloadedRsp& rsp) {
      SWC_LOGF(LOG_DEBUG, "RangeUnloadInternal err=%d(%s) %lu/%lu",
               rsp.err, Error::get_text(rsp.err), cid, rid);
      if(rsp.err && valid() &&
         rsp.err != Error::CLIENT_STOPPING &&
         rsp.err != Error::COLUMN_NOT_EXISTS &&
         rsp.err != Error::COLUMN_MARKED_REMOVED &&
         rsp.err != Error::COLUMN_NOT_READY) {
         req->request_again();
      }
    }
  };

};


}}}
#endif // swcdb_ranger_callbacks_RangeUnloadInternal_h
