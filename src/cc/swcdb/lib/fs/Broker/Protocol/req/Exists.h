/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_req_Exists_h
#define swc_lib_fs_Broker_Protocol_req_Exists_h

#include "Base.h"
#include "../params/Exists.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Exists : public Base {

  public:

  bool  state;

  Exists(const String &name, Callback::ExistsCb_t cb=0) 
        : name(name), cb(cb) {
    HT_DEBUGF("exists path='%s'", name.c_str());

    CommHeader header(Cmd::FUNCTION_EXISTS, 20000);
    Params::ExistsReq params(name);
    cbp = CommBufPtr(new CommBuf(header, params.encoded_length()));
    params.encode(cbp->get_data_ptr_address());
  }

  std::promise<void> promise(){
    std::promise<void>  r_promise;
    cb = [await=&r_promise](int err, bool state){await->set_value();};
    return r_promise;
  }

  void handle(ConnHandlerPtr conn, EventPtr &ev) { 

    const uint8_t *ptr;
    size_t remain;

    if(!Base::is_rsp(conn, ev, Cmd::FUNCTION_EXISTS, &ptr, &remain))
      return;

    if(error != Error::OK) {
      state = false;
      HT_ERRORF("error=%d(%s)", error, 
                SWC::Protocol::string_format_message(ev).c_str());

    } else {
      Params::ExistsRsp params;
      params.decode(&ptr, &remain);
      state = params.get_exists();
    }

    HT_DEBUGF("exists path='%s' error='%d' state='%d'",
               name.c_str(), error, (int)state);
    
    cb(error, state);
  }

  private:
  const String          name;
  Callback::ExistsCb_t  cb;
};
typedef std::shared_ptr<Exists> ExistsPtr;



}}}}

#endif  // swc_lib_fs_Broker_Protocol_req_Exists_h