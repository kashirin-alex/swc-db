/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_rangeserver_AppContextClient_h
#define swc_lib_rangeserver_AppContextClient_h


namespace SWC { namespace client { namespace RS {

class AppContext : public SWC::AppContext {
  public:

  AppContext() {}
  virtual ~AppContext(){}

  void disconnected(ConnHandlerPtr conn) {};

  void handle(ConnHandlerPtr conn, EventPtr ev) override {

    HT_INFOF("handle: %s", ev->to_str().c_str());
    
    switch (ev->type) {

      case Event::Type::CONNECTION_ESTABLISHED: {
        return;
      }
      
      case Event::Type::DISCONNECT:{
        disconnected(conn);
        return;
      }

      case Event::Type::ERROR:{
        switch (ev->header.command) {

          case Protocol::Command::MNGR_REQ_MNGRS_STATE:{
            if(ev->error == Error::Code::REQUEST_TIMEOUT)
            break;
          }
      
          default: {
            break;
          }
        }
        break;
      }

      case Event::Type::MESSAGE: {
      
        switch (ev->header.command) {

          case Protocol::Command::MNGR_REQ_MNGRS_STATE:
            break;
      
          default: {
            break;
          }
          break;
        }

      }

      default: {
        break;
      }

    }
    
    HT_WARNF("unhandled: %s", ev->to_str().c_str());
  
  }
  
};
typedef std::shared_ptr<AppContext> AppContextPtr;

}}}

#endif // swc_lib_rangeserver_AppContextClient_h