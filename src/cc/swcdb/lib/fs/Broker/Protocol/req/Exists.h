/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_req_Exists_h
#define swc_lib_fs_Broker_Protocol_req_Exists_h

#include "../params/Exists.h"

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Exists : public DispatchHandler {

  public:
  CommBufPtr cbp;

  Exists(const String &name, Callback::ExistsCb_t cb=0) 
        : name(name), was_called(false) {
    HT_DEBUGF("exists path='%s'", name.c_str());

    CommHeader header(Cmd::FUNCTION_EXISTS, 20000);
    Params::ExistsReq params(name);
    cbp = CommBufPtr(new CommBuf(header, params.encoded_length()));
    params.encode(cbp->get_data_ptr_address());

    if(cb == 0)
      r_future = r_promise.get_future();
  }

  bool get_result(int &err){
    bool state = r_future.get();
    err = m_err;
    return state;
  }

  void handle(ConnHandlerPtr conn, EventPtr &ev) { 
    HT_DEBUGF("handle: %s", ev->to_str().c_str());

    if(was_called)
      return;

    switch(ev->type){
      case Event::Type::CONNECTION_ESTABLISHED:
        return;
      case Event::Type::DISCONNECT:
        m_err = Error::COMM_NOT_CONNECTED;
        break;
      case Event::Type::ERROR:
        m_err = ev->error;
        break;
      default:
        break;
    }

    const uint8_t *ptr;
    size_t remain;
    
    if(m_err == Error::OK) {
      if(ev->header.command != Cmd::FUNCTION_EXISTS)
        m_err = Error::NOT_IMPLEMENTED;
      else {
        m_err = SWC::Protocol::response_code(ev);
        if(m_err == Error::OK){
          ptr = ev->payload + 4;
          remain = ev->payload_len - 4;
        }
      }
    }

    bool state;
    if(m_err != Error::OK) {
      state = false;
      HT_ERRORF("error=%d(%s)", m_err, 
                SWC::Protocol::string_format_message(ev).c_str());
    } else {
      Params::ExistsRsp params;
      params.decode(&ptr, &remain);
      state = params.get_exists();
    }

    HT_DEBUGF("exists path='%s' err='%d' state='%d'",
               name.c_str(), m_err, (int)state);
    if(cb == 0){
      r_promise.set_value(state);
    } else {
      cb(m_err, state);
    }    
    was_called = true;
  }

  private:
  const String          name;
  int                   m_err=Error::OK;
  Callback::ExistsCb_t  cb;
  std::atomic<bool>     was_called;
  std::promise<bool>    r_promise;
  std::future<bool>     r_future;
};
typedef std::shared_ptr<Exists> ExistsPtr;



}}}}

#endif  // swc_lib_fs_Broker_Protocol_req_Exists_h