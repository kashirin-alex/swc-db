/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_comm_AppContext_h
#define swcdb_core_comm_AppContext_h


#include "swcdb/core/comm/Event.h"
#include "swcdb/core/comm/Resolver.h"


namespace SWC { namespace Comm {

// forward declarations
class ConnHandler;
typedef std::shared_ptr<ConnHandler> ConnHandlerPtr;


class AppContext : public std::enable_shared_from_this<AppContext> {
  public:
  typedef std::shared_ptr<AppContext> Ptr;

  const Config::Property::Value_enum_g::Ptr cfg_encoder;

  SWC_CAN_INLINE
  AppContext(Config::Property::Value_enum_g::Ptr a_cfg_encoder) noexcept
            : cfg_encoder(a_cfg_encoder) {
  }

  virtual void init(const std::string&, const EndPoints&) { }

  virtual void stop();

  virtual void handle_established(ConnHandlerPtr conn) = 0;

  virtual void handle_disconnect(ConnHandlerPtr conn) noexcept = 0;

  virtual void handle(ConnHandlerPtr conn, const Event::Ptr& ev) = 0;

  virtual void net_bytes_sent(const ConnHandlerPtr&, size_t) noexcept { }

  virtual void net_bytes_received(const ConnHandlerPtr&, size_t) noexcept { }

  virtual void net_accepted(const EndPoint&, bool) noexcept { }

  protected:

  virtual ~AppContext() noexcept { };

};


}} // namespace SWC::Comm



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/AppContext.cc"
#endif

#endif // swcdb_core_comm_AppContext_h