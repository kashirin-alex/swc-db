/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_comm_ConfigSSL_h
#define swcdb_core_comm_ConfigSSL_h


#include "swcdb/core/comm/IoContext.h"
#include "swcdb/core/comm/ConnHandler.h"


namespace SWC { namespace Comm {


class ConfigSSL final {
  public:
  typedef std::function<
    void(const ConnHandlerPtr&, const asio::error_code&)> HandshakeCb_t;

  ConfigSSL(const Config::Settings& settings, bool is_client=true);

  //~ConfigSSL() { }

  void set_networks(const Config::Strings& networks, bool with_local);

  SWC_CAN_INLINE
  bool need_ssl(const EndPoint& remote) const noexcept {
    return !Resolver::is_network(remote, nets_v4, nets_v6);
  }

  SWC_CAN_INLINE
  bool need_ssl(const EndPoint& local,
                const EndPoint& remote) const noexcept {
    return local.address() != remote.address() && need_ssl(remote);
  }

  void make_server(AppContext::Ptr& app_ctx,
                   SocketPlain& socket) const;


  ConnHandlerSSL::Ptr
  make_client(AppContext::Ptr& app_ctx, SocketPlain& socket) const;

  ConnHandlerPtr
  make_client(AppContext::Ptr& app_ctx, SocketPlain& socket,
              asio::error_code& ec) const;

  void
  make_client(AppContext::Ptr& app_ctx, SocketPlain& socket,
              HandshakeCb_t&& cb) const;

  private:

  void load_file(const std::string& pathbase,
                 std::string filepath, std::string& to) const;

  Networks_v4                 nets_v4;
  Networks_v6                 nets_v6;
  std::string                 subject_name;
  mutable asio::ssl::context  ctx;

};



}} //namespace SWC::Comm


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/ConfigSSL.cc"
#endif

#endif // swcdb_core_comm_ConfigSSL_h
