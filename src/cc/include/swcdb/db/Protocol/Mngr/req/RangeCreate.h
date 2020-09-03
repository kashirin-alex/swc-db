
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_db_protocol_req_RangeCreate_h
#define swc_db_protocol_req_RangeCreate_h


#include "swcdb/db/Protocol/Commands.h"

#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Mngr/params/RangeCreate.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

  
class RangeCreate: public client::ConnQueue::ReqBase {
  public:
  
  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&, 
                             const Params::RangeCreateRsp&)> Cb_t;
 
  static void request(cid_t cid, rgrid_t rgrid, 
                      const Cb_t& cb, const uint32_t timeout = 10000) {
    request(Params::RangeCreateReq(cid, rgrid), cb, timeout);
  }

  static inline void request(const Params::RangeCreateReq& params,
                             const Cb_t& cb, 
                             const uint32_t timeout = 10000) {
    std::make_shared<RangeCreate>(params, cb, timeout)->run();
  }


  RangeCreate(const Params::RangeCreateReq& params, const Cb_t& cb, 
              const uint32_t timeout) 
              : client::ConnQueue::ReqBase(false), 
                cb(cb), cid(params.cid) {
    cbp = CommBuf::make(params);
    cbp->header.set(RANGE_CREATE, timeout);
  }

  virtual ~RangeCreate() { }

  void handle_no_conn() override {
    clear_endpoints();
    run();
  }

  bool run() override {
    if(endpoints.empty()) {
      Env::Clients::get()->mngrs_groups->select(cid, endpoints);
      if(endpoints.empty()) {
        MngrActive::make(cid, shared_from_this())->run();
        return false;
      }
    } 
    Env::Clients::get()->mngr->get(endpoints)->put(req());
    return true;
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    if(ev->type == Event::Type::DISCONNECT)
      return handle_no_conn();

    Params::RangeCreateRsp rsp_params(ev->error);
    if(!rsp_params.err) {
      try {
        const uint8_t *ptr = ev->data.base;
        size_t remain = ev->data.size;
        rsp_params.decode(&ptr, &remain);

      } catch(...) {
        const Exception& e = SWC_CURRENT_EXCEPTION("");
        SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
        rsp_params.err = e.code();
      }
    }

    cb(req(), rsp_params);
  }

  private:
  
  void clear_endpoints() {
    Env::Clients::get()->mngrs_groups->remove(endpoints);
    endpoints.clear();
  }

  const Cb_t      cb;
  const cid_t     cid;
  EndPoints       endpoints;
};


}}}}

#endif // swc_db_protocol_req_RangeCreate_h
