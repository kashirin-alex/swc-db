/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_req_Length_h
#define swc_lib_fs_Broker_Protocol_req_Length_h

#include "Base.h"
#include "../params/Length.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Length : public Base {

  public:

  size_t length;

  Length(const String &name, Callback::LengthCb_t cb=0) 
        : name(name), cb(cb), length(0) {
    HT_DEBUGF("length path='%s'", name.c_str());

    CommHeader header(Cmd::FUNCTION_LENGTH, 20000);
    Params::LengthReq params(name);
    cbp = CommBufPtr(new CommBuf(header, params.encoded_length()));
    params.encode(cbp->get_data_ptr_address());
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise](int err, size_t len){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, EventPtr &ev) { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_LENGTH, &ptr, &remain))
      return;

    if(error == Error::OK) {
      Params::LengthRsp params;
      params.decode(&ptr, &remain);
      length = params.get_length();
    }

    HT_DEBUGF("length path='%s' error='%d' length='%d'",
               name.c_str(), error, length);
    
    cb(error, length);
  }

  private:
  const String          name;
  Callback::LengthCb_t  cb;
};
typedef std::shared_ptr<Length> LengthPtr;



}}}}

#endif  // swc_lib_fs_Broker_Protocol_req_Length_h