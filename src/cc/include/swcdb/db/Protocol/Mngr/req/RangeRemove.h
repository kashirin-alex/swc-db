/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_req_RangeRemove_h
#define swcdb_db_protocol_req_RangeRemove_h


#include "swcdb/db/Protocol/Commands.h"

#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Mngr/params/RangeRemove.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class RangeRemove: public client::ConnQueue::ReqBase {
  public:

  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&,
                             const Params::RangeRemoveRsp&)> Cb_t;

  static void request(const SWC::client::Clients::Ptr& clients,
                      cid_t cid, rid_t rid,
                      Cb_t&& cb, const uint32_t timeout = 10000) {
    request(
      clients, Params::RangeRemoveReq(cid, rid), std::move(cb), timeout);
  }

  static inline void request(const SWC::client::Clients::Ptr& clients,
                             const Params::RangeRemoveReq& params,
                             Cb_t&& cb, const uint32_t timeout = 10000) {
    std::make_shared<RangeRemove>(
      clients, params, std::move(cb), timeout)->run();
  }


  RangeRemove(const SWC::client::Clients::Ptr& clients,
              const Params::RangeRemoveReq& params, Cb_t&& cb,
              const uint32_t timeout)
              : client::ConnQueue::ReqBase(
                  false,
                  Buffers::make(params, 0, RANGE_REMOVE, timeout)
                ),
                clients(clients), cb(std::move(cb)), cid(params.cid) {
  }

  virtual ~RangeRemove() { }

  void handle_no_conn() override {
    clear_endpoints();
    run();
  }

  bool run() override {
    if(endpoints.empty()) {
      clients->get_mngr(cid, endpoints);
      if(endpoints.empty()) {
        MngrActive::make(clients, cid, shared_from_this())->run();
        return false;
      }
    }
    clients->get_mngr_queue(endpoints)->put(req());
    return true;
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Params::RangeRemoveRsp rsp_params(ev->error);
    if(!rsp_params.err) {
      try {
        const uint8_t *ptr = ev->data.base;
        size_t remain = ev->data.size;
        rsp_params.decode(&ptr, &remain);

      } catch(...) {
        const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
        SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
        rsp_params.err = e.code();
      }
    }

    cb(req(), rsp_params);
  }

  private:

  void clear_endpoints() {
    clients->remove_mngr(endpoints);
    endpoints.clear();
  }

  SWC::client::Clients::Ptr clients;
  const Cb_t                cb;
  const cid_t               cid;
  EndPoints                 endpoints;
};


}}}}}

#endif // swcdb_db_protocol_req_RangeRemove_h
