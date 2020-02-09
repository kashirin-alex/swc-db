/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_fsbroker_AppContext_h
#define swc_app_fsbroker_AppContext_h

#include "swcdb/core/Serialization.h"

#include "swcdb/core/comm/AppContext.h"
#include "swcdb/core/comm/AppHandler.h"

#include "swcdb/fs/Interface.h"
#include "swcdb/fs/Broker/Protocol/Commands.h"
#include "swcdb/fsbroker/FdsMap.h"

#include "swcdb/db/Protocol/Common/handlers/NotImplemented.h"
#include "swcdb/fsbroker/handlers/Exists.h"
#include "swcdb/fsbroker/handlers/Remove.h"
#include "swcdb/fsbroker/handlers/Length.h"
#include "swcdb/fsbroker/handlers/Mkdirs.h"
#include "swcdb/fsbroker/handlers/Readdir.h"
#include "swcdb/fsbroker/handlers/Rmdir.h"
#include "swcdb/fsbroker/handlers/Rename.h"
#include "swcdb/fsbroker/handlers/Write.h"
#include "swcdb/fsbroker/handlers/ReadAll.h"
#include "swcdb/fsbroker/handlers/Create.h"
#include "swcdb/fsbroker/handlers/Append.h"
#include "swcdb/fsbroker/handlers/Open.h"
#include "swcdb/fsbroker/handlers/Read.h"
#include "swcdb/fsbroker/handlers/Pread.h"
#include "swcdb/fsbroker/handlers/Seek.h"
#include "swcdb/fsbroker/handlers/Flush.h"
#include "swcdb/fsbroker/handlers/Sync.h"
#include "swcdb/fsbroker/handlers/Close.h"


namespace SWC { namespace server { namespace FsBroker {


class AppContext : public SWC::AppContext {
  
  // in-order of FS::Protocol::Cmd
  static constexpr const AppHandler_t handlers[] = { 
    &Protocol::Common::Handler::not_implemented,
    &Handler::open,
    &Handler::create,
    &Handler::close,
    &Handler::read,
    &Handler::append,
    &Handler::seek,
    &Handler::remove,
    &Handler::length,
    &Handler::pread,
    &Handler::mkdirs,
    &Handler::flush,
    &Handler::rmdir,
    &Handler::readdir,
    &Handler::exists,
    &Handler::rename,
    &Handler::sync,
    &Handler::write,
    &Handler::read_all

    //&Handler::debug,
    //&Handler::status,
    //&Handler::shutdown
  }; 

  public:

  AppContext() {
    Env::IoCtx::init(
      Env::Config::settings()->get<int32_t>("swc.FsBroker.handlers"));
    Env::FsInterface::init(FS::fs_type(
      Env::Config::settings()->get<std::string>("swc.fs.broker.underlying")));
    Env::Fds::init();
  }
  
  void init(const EndPoints& endpoints) override {
    int sig = 0;
    Env::IoCtx::io()->set_signals();
    shutting_down(std::error_code(), sig);
  }

  void set_srv(SerializedServer::Ptr srv){
    m_srv = srv;
  }

  virtual ~AppContext(){}

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override {
    //SWC_LOGF(LOG_DEBUG, "handle: %s", ev->to_str().c_str());

    switch (ev->type) {

      case Event::Type::ESTABLISHED:
        m_srv->connection_add(conn);
        return; 
        
      case Event::Type::DISCONNECT:
        m_srv->connection_del(conn);
        return;

      case Event::Type::ERROR:
        break;

      case Event::Type::MESSAGE: {
      uint8_t cmd = ev->header.command >= FS::Protocol::Cmd::FUNCTION_MAX 
                    ? FS::Protocol::Cmd::NOT_IMPLEMENTED : ev->header.command;
        asio::post(
          *Env::IoCtx::io()->ptr(), 
          [cmd, conn, ev]() { 
            handlers[cmd](conn, ev); 
          }
        );
        return;
      }

      default:
        SWC_LOGF(LOG_WARN, "Unimplemented event-type (%llu)", (Llu)ev->type);
        break;

    }
  }
  
  void shutting_down(const std::error_code &ec, const int &sig) {

    if(sig==0){ // set signals listener
      Env::IoCtx::io()->signals()->async_wait(
        [ptr=this](const std::error_code &ec, const int &sig){
          SWC_LOGF(LOG_INFO, "Received signal, sig=%d ec=%s", sig, ec.message().c_str());
          ptr->shutting_down(ec, sig); 
        }
      ); 
      SWC_LOGF(LOG_INFO, "Listening for Shutdown signal, set at sig=%d ec=%s", 
              sig, ec.message().c_str());
      return;
    }

    SWC_LOGF(LOG_INFO, "Shutdown signal, sig=%d ec=%s", sig, ec.message().c_str());
    (new std::thread([ptr=shared_from_this()]{ ptr->stop(); }))->detach();
  }

  void stop() override {
     
    m_srv->stop_accepting(); // no further requests accepted

    FS::SmartFd::Ptr fd;
    int err;
    while((fd = Env::Fds::get()->pop_next()) != nullptr){
      if(fd->flags() & O_WRONLY)
        Env::FsInterface::fs()->sync(err, fd);
      Env::FsInterface::fs()->close(err, fd);
    }
    
    Env::FsInterface::interface()->stop();
    
    Env::IoCtx::io()->stop();
    
    m_srv->shutdown();

    SWC_LOG(LOG_INFO, "Exit");
    std::quick_exit(0);
  }

  private:
  SerializedServer::Ptr m_srv = nullptr;
};

}}}

#endif // swc_app_fsbroker_AppContext_h