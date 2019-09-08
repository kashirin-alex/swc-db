/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_manager_AppContextClient_h
#define swc_lib_manager_AppContextClient_h

namespace SWC { namespace client { namespace Mngr { 

class AppContext : public SWC::AppContext {
  public:

  AppContext() {}
  virtual ~AppContext(){}

  void disconnected(ConnHandlerPtr conn);

  void handle(ConnHandlerPtr conn, EventPtr ev) override {

    switch (ev->type) {

      case Event::Type::CONNECTION_ESTABLISHED: {
        break;
      }
      
      case Event::Type::DISCONNECT: {
        disconnected(conn);
        return;
      }

      case Event::Type::ERROR: {
        HT_WARNF("unhandled: %s", ev->to_str().c_str());
        break;
      }

      case Event::Type::MESSAGE: {
        HT_WARNF("unhandled: %s", ev->to_str().c_str());
        break;
      }

      default: {
        break;
      }

    }
    
  }
  
};

typedef std::shared_ptr<AppContext> AppContextPtr;
}}}

#endif // swc_lib_client_AppContextClient_h