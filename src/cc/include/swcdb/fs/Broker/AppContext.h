/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_AppContext_h
#define swcdb_fs_Broker_AppContext_h

#include "swcdb/core/comm/AppContext.h"

namespace SWC { namespace client {



/**
 * @brief The SWC-DB FS::Broker's Client to FsBroker C++ namespace 'SWC::client::FsBroker'
 *
 * \ingroup FileSystem
 */
namespace FsBroker {


class AppContext final : public Comm::AppContext {
  public:

  typedef std::shared_ptr<AppContext> Ptr;

  AppContext(const Config::Settings& settings)
      : Comm::AppContext(
          settings.get<Config::Property::V_GENUM>(
            "swc.fs.broker.comm.encoder")) {
  }

  virtual ~AppContext() noexcept { }

  void handle_established(Comm::ConnHandlerPtr) override { }

  void handle_disconnect(Comm::ConnHandlerPtr) noexcept override { }

  void handle(Comm::ConnHandlerPtr, const Comm::Event::Ptr& ev) override {
    SWC_LOG_OUT(LOG_WARN,  ev->print(SWC_LOG_OSTREAM << "Unhandled "); );
  }

};

}}}

#endif // swcdb_fs_Broker_AppContext_h
