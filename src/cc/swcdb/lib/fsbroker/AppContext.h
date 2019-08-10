/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_fsbroker_AppContext_h
#define swc_app_fsbroker_AppContext_h

#include "swcdb/lib/core/comm/AppContext.h"
#include "swcdb/lib/core/comm/AppHandler.h"

#include "swcdb/lib/fs/Interface.h"
#include "swcdb/lib/fs/Broker/Protocol/Commands.h"
#include "FdsMap.h"
#include "handlers/Exists.h"
#include "handlers/Remove.h"
#include "handlers/Length.h"
#include "handlers/Mkdirs.h"
#include "handlers/Readdir.h"
#include "handlers/Rmdir.h"
#include "handlers/Create.h"
#include "handlers/Append.h"
#include "handlers/Open.h"
#include "handlers/Read.h"
#include "handlers/Close.h"


namespace SWC { namespace server { namespace FsBroker {


class AppContext : public SWC::AppContext {
  
  public:

  AppContext() {
    EnvIoCtx::init(
      EnvConfig::settings()->get<int32_t>("swc.FsBroker.handlers"));
    EnvFsInterface::init();
    EnvFds::init();
  }
  
  void init(EndPoints endpoints) override {}

  void set_srv(SerializedServerPtr srv){
    m_srv = srv;
  }

  virtual ~AppContext(){}

  void handle(ConnHandlerPtr conn, EventPtr ev) override {
    HT_DEBUGF("handle: %s", ev->to_str().c_str());

    switch (ev->type) {

      case Event::Type::CONNECTION_ESTABLISHED:
        return;

      case Event::Type::DISCONNECT:
        break;

      case Event::Type::ERROR:
        break;

      case Event::Type::MESSAGE: {
        
        AppHandler *handler = 0;
        switch (ev->header.command) {

          case FS::Protocol::Cmd::FUNCTION_EXISTS:
            handler = new Handler::Exists(conn, ev);
            break;

          case FS::Protocol::Cmd::FUNCTION_REMOVE:
            handler = new Handler::Remove(conn, ev);
            break;

          case FS::Protocol::Cmd::FUNCTION_LENGTH:
            handler = new Handler::Length(conn, ev);
            break;

          case FS::Protocol::Cmd::FUNCTION_MKDIRS:
            handler = new Handler::Mkdirs(conn, ev);
            break;

          case FS::Protocol::Cmd::FUNCTION_READDIR:
            handler = new Handler::Readdir(conn, ev);
            break;

          case FS::Protocol::Cmd::FUNCTION_RMDIR:
            handler = new Handler::Rmdir(conn, ev);
            break;

          case FS::Protocol::Cmd::FUNCTION_CREATE:
            handler = new Handler::Create(conn, ev);
            break;

          case FS::Protocol::Cmd::FUNCTION_APPEND:
            handler = new Handler::Append(conn, ev);
            break;

          case FS::Protocol::Cmd::FUNCTION_OPEN:
            handler = new Handler::Open(conn, ev);
            break;

          case FS::Protocol::Cmd::FUNCTION_READ:
            handler = new Handler::Read(conn, ev);
            break;

          case FS::Protocol::Cmd::FUNCTION_CLOSE:
            handler = new Handler::Close(conn, ev);
            break;

          default: {
            conn->send_error(Error::NOT_IMPLEMENTED, 
              format("event command (%llu)",(Llu)ev->header.command), ev);
          }
        }

        if(handler)
          asio::post(*EnvIoCtx::io()->ptr(), [handler](){ handler->run();  });

        break;
      }

      default:
        conn->send_error(Error::NOT_IMPLEMENTED, 
          format("event-type (%llu)",(Llu)ev->type), ev);

    }
  }
  
  void shutting_down(const std::error_code &ec, const int &sig) {

    if(sig==0){ // set signals listener
      EnvIoCtx::io()->signals()->async_wait(
        [ptr=this](const std::error_code &ec, const int &sig){
          HT_INFOF("Received signal, sig=%d ec=%s", sig, ec.message().c_str());
          ptr->shutting_down(ec, sig); 
        }
      ); 
      HT_INFOF("Listening for Shutdown signal, set at sig=%d ec=%s", 
              sig, ec.message().c_str());
      return;
    }

    HT_INFOF("Shutdown signal, sig=%d ec=%s", sig, ec.message().c_str());
    stop();
  }

  void stop(){
    HT_INFO("Stopping APP-FSBROKER");
    
    m_srv->stop_accepting(); // no further requests accepted
    
    // + close all fds
    
    EnvIoCtx::io()->stop();
    EnvFsInterface::fs()->stop();
    
    m_srv->shutdown();
  }

  private:
  SerializedServerPtr m_srv = nullptr;
};

}}}

#endif // swc_app_fsbroker_AppContext_h