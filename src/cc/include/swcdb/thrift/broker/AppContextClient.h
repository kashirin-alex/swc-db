/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_ThriftBroker_AppContextClient_h
#define swc_ThriftBroker_AppContextClient_h


namespace SWC { namespace client { namespace ThriftBroker {

class AppContext final : public SWC::AppContext {
  public:

  AppContext() {}
  virtual ~AppContext(){}

  void disconnected(const ConnHandlerPtr&) {};

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override {
    
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

#endif // swc_ThriftBroker_AppContextClient_h