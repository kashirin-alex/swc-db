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

      case Event::Type::DISCONNECT: {
        disconnected(conn);
        return;
      }

      case Event::Type::MESSAGE:
      case Event::Type::ERROR: {
        SWC_LOG_OUT(LOG_WARN,  ev->print(SWC_LOG_OSTREAM << "unhandled: "); );
        break;
      }

      case Event::Type::ESTABLISHED:
      default: {
        break;
      }

    }
    
  
  }
  
};

}}}

#endif // swc_ThriftBroker_AppContextClient_h