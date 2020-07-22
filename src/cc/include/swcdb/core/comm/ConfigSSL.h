/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_core_comm_ConfigSSL_h
#define swc_core_comm_ConfigSSL_h

#include <string>
#include <vector>
#include <memory>
#include "swcdb/core/comm/IoContext.h"
#include "swcdb/core/comm/ConnHandler.h"

namespace SWC {


class ConfigSSL final {
  public:
  typedef std::function<
    void(const ConnHandlerPtr&, const asio::error_code&)> HandshakeCb_t;

  ConfigSSL(bool is_client=true);

  ~ConfigSSL();

  void set_networks(const Strings& networks);

  bool need_ssl(const EndPoint& endpoint) const;


  void make_server(AppContext::Ptr& app_ctx,
                   SocketPlain& socket) const;


  std::shared_ptr<ConnHandlerSSL> 
  make_client(AppContext::Ptr& app_ctx, SocketPlain& socket) const;

  std::shared_ptr<ConnHandlerSSL> 
  make_client(AppContext::Ptr& app_ctx, SocketPlain& socket,
              asio::error_code& ec) const;

  void 
  make_client(AppContext::Ptr& app_ctx, SocketPlain& socket, 
              const HandshakeCb_t& cb) const;

  private:
  
  void load_file(std::string filepath, std::string& to) const;
  
  std::string   ciphers;
  std::string   subject_name;
  std::string   crt;
  std::string   key;
  std::string   ca;
  std::vector<asio::ip::network_v4> nets_v4;
  std::vector<asio::ip::network_v6> nets_v6;
};



}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/ConfigSSL.cc"
#endif 

#endif // swc_core_comm_ConfigSSL_h