/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_AppContextClient_h
#define swcdb_ranger_AppContextClient_h


namespace SWC { namespace client { namespace Rgr {

class AppContext final : public SWC::Comm::AppContext {
  public:

  AppContext() {}
  virtual ~AppContext(){}

  void disconnected(const Comm::ConnHandlerPtr&) {};

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override {
    
    switch (ev->type) {

      case Comm::Event::Type::DISCONNECT: {
        disconnected(conn);
        return;
      }

      case Comm::Event::Type::ERROR: 
      case Comm::Event::Type::MESSAGE: {
        SWC_LOG_OUT(LOG_WARN,  ev->print(SWC_LOG_OSTREAM << "unhandled: "); );
        break;
      }

      case Comm::Event::Type::ESTABLISHED:
      default: {
        break;
      }

    }
    
  
  }
  
};

}}}

#endif // swcdb_ranger_AppContextClient_h