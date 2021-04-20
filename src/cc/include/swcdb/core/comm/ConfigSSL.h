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

  ConfigSSL(bool is_client=true);

  //~ConfigSSL() { }

  void set_networks(const Config::Strings& networks, bool with_local);

  bool need_ssl(const EndPoint& endpoint) const noexcept;


  void make_server(AppContext::Ptr& app_ctx,
                   SocketPlain& socket) const;


  std::shared_ptr<ConnHandlerSSL>
  make_client(AppContext::Ptr& app_ctx, SocketPlain& socket) const;

  ConnHandlerPtr
  make_client(AppContext::Ptr& app_ctx, SocketPlain& socket,
              asio::error_code& ec) const;

  void
  make_client(AppContext::Ptr& app_ctx, SocketPlain& socket,
              HandshakeCb_t&& cb) const;

  private:

  void load_file(std::string filepath, std::string& to) const;

  std::vector<asio::ip::network_v4> nets_v4;
  std::vector<asio::ip::network_v6> nets_v6;
  std::string                       subject_name;
  mutable asio::ssl::context        ctx;

};



}} //namespace SWC::Comm


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/ConfigSSL.cc"
#endif

#endif // swcdb_core_comm_ConfigSSL_h
