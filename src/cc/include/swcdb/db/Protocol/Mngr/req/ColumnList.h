
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_mngr_req_ColumnList_h
#define swc_db_protocol_mngr_req_ColumnList_h


#include "swcdb/db/Protocol/Commands.h"

#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"
#include "swcdb/db/Protocol/Mngr/params/ColumnList.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

  
class ColumnList: public Common::Req::ConnQueue::ReqBase {
  public:
  
  typedef std::function<void(Common::Req::ConnQueue::ReqBase::Ptr, 
                             int, Params::ColumnListRsp)> Cb_t;

  static void request(const Cb_t cb, const uint32_t timeout = 10000){
    std::make_shared<ColumnList>(
      Params::ColumnListReq(), cb, timeout
    )->run();
  }

  ColumnList(const Params::ColumnListReq& params, const Cb_t cb, 
             const uint32_t timeout) 
            : Common::Req::ConnQueue::ReqBase(false), cb(cb) {
    cbp = CommBuf::make(params);
    cbp->header.set(COLUMN_LIST, timeout);
  }

  virtual ~ColumnList(){}

  void handle_no_conn() override {
    clear_endpoints();
    run();
  }

  bool run(uint32_t timeout=0) override {
    if(endpoints.empty()){
      // columns-get (can be any mngr)
      Env::Clients::get()->mngrs_groups->select(1, endpoints); 
      if(endpoints.empty()){
        std::make_shared<MngrActive>(1, shared_from_this())->run();
        return false;
      }
    } 
    Env::Clients::get()->mngr->get(endpoints)->put(req());
    return true;
  }

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override {

    if(ev->type == Event::Type::DISCONNECT){
      handle_no_conn();
      return;
    }

    Params::ColumnListRsp rsp_params;
    int err = ev->error != Error::OK? ev->error: ev->response_code();

    if(err == Error::OK){
      try{
        const uint8_t *ptr = ev->data.base+4;
        size_t remain = ev->data.size-4;
        rsp_params.decode(&ptr, &remain);
      } catch (Exception &e) {
        SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
        err = e.code();
      }
    }

    cb(req(), err, rsp_params);
  }

  private:
  
  void clear_endpoints() {
    Env::Clients::get()->mngrs_groups->remove(endpoints);
    endpoints.clear();
  }

  const Cb_t  cb;
  EndPoints   endpoints;
};



}}}}

#endif // swc_db_protocol_mngr_req_ColumnList_h
