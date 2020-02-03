/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
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
    void(ConnHandlerPtr, const asio::error_code&)> HandshakeCb_t;

  ConfigSSL(bool is_client=true);

  ~ConfigSSL();

  void set_networks(const Strings& networks);

  void load_ca(const std::string& ca_filepath);


  const bool need_ssl(const EndPoint& endpoint) const;


  void configure_server(asio::ssl::context& ctx) const;
  
  void make_server(AppContext::Ptr& app_ctx, SocketPlain& socket);


  void configure_client(asio::ssl::context& ctx) const;

  std::shared_ptr<ConnHandlerSSL> make_client(AppContext::Ptr& app_ctx, 
                                              SocketPlain& socket, 
                                              asio::error_code& ec);

  void make_client(AppContext::Ptr& app_ctx, SocketPlain& socket, 
                   const HandshakeCb_t& cb);

  bool verify(bool preverified, asio::ssl::verify_context& ctx);

  private:
  
  std::string   ssl_pem;
  std::string   ssl_ciphers;
  std::string   subject_name;
  std::string   ca;
  std::string   ca_file;
  std::vector<asio::ip::network_v4> nets_v4;
  std::vector<asio::ip::network_v6> nets_v6;
};



}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/ConfigSSL.cc"
#endif 

#endif // swc_core_comm_ConfigSSL_h