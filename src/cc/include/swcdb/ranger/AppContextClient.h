/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_ranger_AppContextClient_h
#define swc_ranger_AppContextClient_h


namespace SWC { namespace client { namespace Rgr {

class AppContext final : public SWC::AppContext {
  public:

  AppContext() {}
  virtual ~AppContext(){}

  void disconnected(ConnHandlerPtr conn) {};

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override {
    
    switch (ev->type) {

      case Event::Type::ESTABLISHED: {
        return;
      }
      
      case Event::Type::DISCONNECT:{
        disconnected(conn);
        return;
      }

      case Event::Type::ERROR:{
        SWC_LOGF(LOG_WARN, "unhandled: %s", ev->to_str().c_str());
        break;
      }

      case Event::Type::MESSAGE: {
        SWC_LOGF(LOG_WARN, "unhandled: %s", ev->to_str().c_str());
        break;
      }

      default: {
        break;
      }

    }
    
  
  }
  
};

}}}

#endif // swc_ranger_AppContextClient_h