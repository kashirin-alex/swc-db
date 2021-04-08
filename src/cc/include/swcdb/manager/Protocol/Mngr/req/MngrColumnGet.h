/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#ifndef swcdb_manager_Protocol_mngr_req_MngrColumnGet_h
#define swcdb_manager_Protocol_mngr_req_MngrColumnGet_h

#include "swcdb/db/Protocol/Mngr/params/ColumnGet.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {

class MngrColumnGet : public client::ConnQueue::ReqBase {
  public:
  
  typedef std::function<void(int, const Params::ColumnGetRsp&)> Cb_t;

  MngrColumnGet(const Params::ColumnGetReq& params, const Cb_t& cb) 
                : client::ConnQueue::ReqBase(
                    true,
                    Buffers::make(params, 0, COLUMN_GET, 60000)
                  ), cb(cb) {
  }
  
  virtual ~MngrColumnGet() { }
  
  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    if(!is_rsp(ev))
      return;

    Params::ColumnGetRsp rsp_params;
    int err = ev->response_code();
    if(!err) {
      try {
        const uint8_t *ptr = ev->data.base + 4;
        size_t remain = ev->data.size - 4;
        rsp_params.decode(&ptr, &remain);

      } catch(...) {
        const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
        SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
        err = e.code();
      }
    }

    cb(err, rsp_params);
  }

  private:
  Cb_t   cb;
  
};

}}}}}

#endif // swcdb_manager_Protocol_mngr_req_MngrColumnGet_h
