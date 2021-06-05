/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_req_RangeUnloaded_h
#define swcdb_db_protocol_req_RangeUnloaded_h


#include "swcdb/db/Protocol/Commands.h"

#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Mngr/params/RangeUnloaded.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class RangeUnloaded: public client::ConnQueue::ReqBase {
  public:

  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&,
                             const Params::RangeUnloadedRsp&)> Cb_t;

  static void request(const SWC::client::Clients::Ptr& clients,
                      cid_t cid, rid_t rid,
                      Cb_t&& cb, const uint32_t timeout = 10000){
    request(
      clients, Params::RangeUnloadedReq(cid, rid), std::move(cb), timeout);
  }

  static inline void request(const SWC::client::Clients::Ptr& clients,
                             const Params::RangeUnloadedReq& params,
                             Cb_t&& cb, const uint32_t timeout = 10000) {
    std::make_shared<RangeUnloaded>(
      clients, params, std::move(cb), timeout)->run();
  }


  RangeUnloaded(const SWC::client::Clients::Ptr& clients,
                const Params::RangeUnloadedReq& params, Cb_t&& cb,
                const uint32_t timeout)
                : client::ConnQueue::ReqBase(
                    false,
                    Buffers::make(params, 0, RANGE_UNLOADED, timeout)
                  ),
                  clients(clients), cb(std::move(cb)), cid(params.cid) {
  }

  virtual ~RangeUnloaded() { }

  void handle_no_conn() override {
    if(clients->stopping()) {
      cb(req(), Params::RangeUnloadedRsp(Error::CLIENT_STOPPING));
    } else {
      clear_endpoints();
      run();
    }
  }

  bool run() override {
    if(endpoints.empty()) {
      clients->get_mngr(cid, endpoints);
      if(endpoints.empty()) {
        if(clients->stopping()) {
          cb(req(), Params::RangeUnloadedRsp(Error::CLIENT_STOPPING));
        } else {
          MngrActive::make(clients, cid, shared_from_this())->run();
        }
        return false;
      }
    }
    clients->get_mngr_queue(endpoints)->put(req());
    return true;
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Params::RangeUnloadedRsp rsp_params(ev->error);
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

#endif // swcdb_db_protocol_req_RangeUnloaded_h
