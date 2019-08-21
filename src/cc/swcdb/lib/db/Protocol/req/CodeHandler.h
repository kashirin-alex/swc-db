
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_rsp_CodeHandler_h
#define swc_lib_db_protocol_rsp_CodeHandler_h

namespace SWC {
namespace Protocol {
namespace Req {


class CodeHandler : public DispatchHandler {
  public:

  struct Req {
    public:
    
    typedef std::shared_ptr<Req> Ptr;
    typedef std::function<void(Ptr, int)> Cb_t;

    Req() {}
    Req(Cb_t cb) : cb(cb) {}

    virtual ~Req(){}

    Cb_t cb;
  };

  CodeHandler(Req::Ptr req) : req(req), was_called(false) {}
  
  virtual ~CodeHandler(){}

  void handle(ConnHandlerPtr conn, EventPtr &ev) override {
    if(was_called)
      return;
    was_called = true;
    
    req->cb(req, ev->error != Error::OK? ev->error: Protocol::response_code(ev));
  }

  Req::Ptr              req;
  std::atomic<bool>     was_called;
};

}}}

#endif // swc_lib_db_protocol_rsp_CodeHandler_h
