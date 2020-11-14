/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fsbroker_AppContext_h
#define swcdb_fsbroker_AppContext_h

#include "swcdb/core/Serialization.h"

#include "swcdb/core/comm/AppContext.h"
#include "swcdb/core/comm/AppHandler.h"

#include "swcdb/fs/Interface.h"
#include "swcdb/fs/Broker/Protocol/Commands.h"
#include "swcdb/fsbroker/FsBrokerEnv.h"

#include "swcdb/common/Protocol/handlers/NotImplemented.h"
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


namespace SWC { namespace FsBroker {


class AppContext final : public Comm::AppContext {
  
  // in-order of Protocol::FsBroker::Coomand
  static constexpr const Comm::AppHandler_t handlers[] = {
    &Comm::Protocol::Common::Handler::not_implemented,
    &Comm::Protocol::FsBroker::Handler::open,
    &Comm::Protocol::FsBroker::Handler::create,
    &Comm::Protocol::FsBroker::Handler::close,
    &Comm::Protocol::FsBroker::Handler::read,
    &Comm::Protocol::FsBroker::Handler::append,
    &Comm::Protocol::FsBroker::Handler::seek,
    &Comm::Protocol::FsBroker::Handler::remove,
    &Comm::Protocol::FsBroker::Handler::length,
    &Comm::Protocol::FsBroker::Handler::pread,
    &Comm::Protocol::FsBroker::Handler::mkdirs,
    &Comm::Protocol::FsBroker::Handler::flush,
    &Comm::Protocol::FsBroker::Handler::rmdir,
    &Comm::Protocol::FsBroker::Handler::readdir,
    &Comm::Protocol::FsBroker::Handler::exists,
    &Comm::Protocol::FsBroker::Handler::rename,
    &Comm::Protocol::FsBroker::Handler::sync,
    &Comm::Protocol::FsBroker::Handler::write,
    &Comm::Protocol::FsBroker::Handler::read_all

    //&Comm::Protocol::FsBroker::Handler::debug,
    //&Comm::Protocol::FsBroker::Handler::status,
    //&Comm::Protocol::FsBroker::Handler::shutdown
  }; 

  public:

  AppContext() 
      : Comm::AppContext(
          Env::Config::settings()->get<Config::Property::V_GENUM>(
            "swc.FsBroker.comm.encoder")) {

    auto settings = Env::Config::settings();

    Env::IoCtx::init(settings->get_i32("swc.FsBroker.handlers"));

    auto fs_type = FS::fs_type(settings->get_str("swc.fs.broker.underlying"));
    SWC_ASSERT(fs_type != FS::Type::BROKER);
    Env::FsInterface::init(fs_type);

    Env::FsBroker::init();
    
    auto period = settings->get<Config::Property::V_GINT32>(
      "swc.cfg.dyn.period");
    if(period->get()) {
      Env::IoCtx::io()->set_periodic_timer(
        period,
        [](){Env::Config::settings()->check_dynamic_files();}
      );
    }
  }
  
  void init(const Comm::EndPoints&) override {
    int sig = 0;
    Env::IoCtx::io()->set_signals();
    shutting_down(std::error_code(), sig);
  }

  void set_srv(Comm::server::SerializedServer::Ptr srv){
    m_srv = srv;
  }

  virtual ~AppContext(){}

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override {
    //SWC_LOGF(LOG_DEBUG, "handle: %s", ev->to_str().c_str());

    switch (ev->type) {

      case Comm::Event::Type::ESTABLISHED:
        m_srv->connection_add(conn);
        return; 
        
      case Comm::Event::Type::DISCONNECT:
        m_srv->connection_del(conn);
        return;

      case Comm::Event::Type::ERROR:
        break;

      case Comm::Event::Type::MESSAGE: {
        uint8_t cmd = ev->header.command >= Comm::Protocol::FsBroker::MAX_CMD
                        ? (uint8_t)Comm::Protocol::FsBroker::NOT_IMPLEMENTED
                        : ev->header.command;
        Env::IoCtx::post([cmd, conn, ev]() { handlers[cmd](conn, ev); });
        return;
      }

      default:
        SWC_LOGF(LOG_WARN, "Unimplemented event-type (%d)", (int)ev->type);
        break;

    }
  }
  
  void shutting_down(const std::error_code &ec, const int &sig) {
    if(!sig) { // set signals listener
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

    int err;
    for(FS::SmartFd::Ptr fd; (fd = Env::FsBroker::fds()->pop_next()); ) {
      if(fd->valid()) {
        err = Error::OK;
        if(fd->flags() & O_WRONLY)
          Env::FsInterface::fs()->sync(err, fd);
        Env::FsInterface::fs()->close(err, fd);
      }
    }
    
    Env::FsInterface::interface()->stop();
    
    Env::IoCtx::io()->stop();
    
    m_srv->shutdown();

    SWC_LOG(LOG_INFO, "Exit");
    std::quick_exit(0);
  }

  private:
  Comm::server::SerializedServer::Ptr m_srv = nullptr;
};


}}



#endif // swcdb_fsbroker_AppContext_h
