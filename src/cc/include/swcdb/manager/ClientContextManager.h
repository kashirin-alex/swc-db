/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_ClientContextManager_h
#define swcdb_manager_ClientContextManager_h

namespace SWC { namespace client {


//! The SWC-DB Manager's Client to Database C++ namespace 'SWC::client::Mngr'
namespace Mngr {


class ContextManager final : public client::ContextManager {
  public:

  ContextManager() : client::ContextManager(*Env::Config::settings()) { }

  virtual ~ContextManager() { }

  void handle_disconnect(Comm::ConnHandlerPtr) noexcept override;

};

}}}

#endif // swcdb_manager_ClientContextManager_h
