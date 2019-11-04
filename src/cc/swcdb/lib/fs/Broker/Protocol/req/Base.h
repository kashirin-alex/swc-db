/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_Protocol_req_Base_h
#define swc_lib_fs_Broker_Protocol_req_Base_h


namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Base : public DispatchHandler {

  public:

  CommBuf::Ptr cbp;
  int          error;
  bool    was_called;

  Base() : error(Error::OK), was_called(false) {}

  virtual ~Base() {}

  bool is_rsp(ConnHandlerPtr conn, Event::Ptr &ev, int cmd, 
              const uint8_t **ptr, size_t *remain) { 
    // HT_DEBUGF("handle: %s", ev->to_str().c_str());

    if(was_called)
      return false;

    switch(ev->type){
      case Event::Type::DISCONNECT:
        error = Error::COMM_NOT_CONNECTED;
        break;
      case Event::Type::ERROR:
        error = ev->error;
        break;
      case Event::Type::ESTABLISHED:
        return false;
      default:
        break;
    }

    if(error == Error::OK && ev->header.command != cmd){
      error = Error::NOT_IMPLEMENTED;
      HT_ERRORF("error=%d(%s) cmd=%d", 
                error, Error::get_text(error), ev->header.command);

    } else if(error == Error::OK 
              && ((error = SWC::Protocol::response_code(ev)) == Error::OK) 
                  || error == Error::FS_EOF) {
      *ptr = ev->data.base + 4;
      *remain = ev->data.size - 4;
    } 
    
    if(error != Error::OK)
      HT_ERRORF("error=%d(%s)", error, Error::get_text(error));
    
    was_called = true;
    return true;
  }

};
typedef std::shared_ptr<Base> ReqBasePtr;



}}}}

#endif  // swc_lib_fs_Broker_Protocol_req_Base_h