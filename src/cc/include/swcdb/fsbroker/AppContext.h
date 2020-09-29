/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fsbroker_AppContext_h
#define swc_fsbroker_AppContext_h

#include "swcdb/core/Serialization.h"

#include "swcdb/core/comm/AppContext.h"
#include "swcdb/core/comm/AppHandler.h"

#include "swcdb/fs/Interface.h"
#include "swcdb/fs/Broker/Protocol/Commands.h"
#include "swcdb/fsbroker/FdsMap.h"

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


class AppContext final : public SWC::Comm::AppContext {
  
  // in-order of FsBroker::Protocol::Cmd
  static constexpr const Comm::AppHandler_t handlers[] = { 
    &SWC::Protocol::Common::Handler::not_implemented,
    &Protocol::Handler::open,
    &Protocol::Handler::create,
    &Protocol::Handler::close,
    &Protocol::Handler::read,
    &Protocol::Handler::append,
    &Protocol::Handler::seek,
    &Protocol::Handler::remove,
    &Protocol::Handler::length,
    &Protocol::Handler::pread,
    &Protocol::Handler::mkdirs,
    &Protocol::Handler::flush,
    &Protocol::Handler::rmdir,
    &Protocol::Handler::readdir,
    &Protocol::Handler::exists,
    &Protocol::Handler::rename,
    &Protocol::Handler::sync,
    &Protocol::Handler::write,
    &Protocol::Handler::read_all

    //&Protocol::Handler::debug,
    //&Protocol::Handler::status,
    //&Protocol::Handler::shutdown
  }; 

  public:

  AppContext() {
    auto settings = Env::Config::settings();

    Env::IoCtx::init(settings->get_i32("swc.FsBroker.handlers"));

    auto fs_type = FS::fs_type(settings->get_str("swc.fs.broker.underlying"));
    SWC_ASSERT(fs_type != Types::Fs::BROKER);
    Env::FsInterface::init(fs_type);

    Env::Fds::init();
    
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
      uint8_t cmd = ev->header.command >= Protocol::Cmd::FUNCTION_MAX 
                      ? (uint8_t)Protocol::Cmd::NOT_IMPLEMENTED 
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

    int err;
    for(FS::SmartFd::Ptr fd; (fd = Env::Fds::get()->pop_next()); ) {
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



#endif // swc_fsbroker_AppContext_h
