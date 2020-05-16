/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/core/FileUtils.h"
#include "swcdb/core/comm/ConfigSSL.h"
#include "swcdb/core/comm/Settings.h"

namespace SWC { 

  
ConfigSSL::ConfigSSL(bool is_client) {
  auto settings = Env::Config::settings();

  set_networks(settings->get_strs("swc.comm.ssl.secure.network"));
  
  ciphers = settings->get_str("swc.comm.ssl.ciphers", "");
  subject_name = settings->get_str("swc.comm.ssl.subject_name", "");

  if(!is_client) {
    load_file(settings->get_str("swc.comm.ssl.crt"), crt);
    load_file(settings->get_str("swc.comm.ssl.key"), key);
  }

  if(settings->has("swc.comm.ssl.ca")) 
    load_file(settings->get_str("swc.comm.ssl.ca"), ca);
}

ConfigSSL::~ConfigSSL() { }


void ConfigSSL::set_networks(const Strings& networks) {
  asio::error_code ec;
  Resolver::get_networks(networks, nets_v4, nets_v6, ec);
  if(ec)
    SWC_THROWF(Error::CONFIG_BAD_VALUE,
              "Bad Network in swc.comm.ssl.secure.network error(%s)",
              ec.message().c_str());
}


SWC_SHOULD_INLINE
bool ConfigSSL::need_ssl(const EndPoint& endpoint) const {
  return !Resolver::is_network(endpoint, nets_v4, nets_v6);
}


void ConfigSSL::configure_server(asio::ssl::context& ctx) const {
  ctx.set_options(//asio::ssl::context::default_workarounds | 
      asio::ssl::context::no_compression
    | asio::ssl::context::no_sslv2
    | asio::ssl::context::no_sslv3
    | asio::ssl::context::no_tlsv1
    | asio::ssl::context::no_tlsv1_1
    | asio::ssl::context::no_tlsv1_2
    | asio::ssl::context::single_dh_use
    | SSL_OP_NO_TICKET
    | SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION
    | SSL_OP_CIPHER_SERVER_PREFERENCE
    | SSL_OP_PRIORITIZE_CHACHA
  );

  if(!ciphers.empty())
    SSL_CTX_set_cipher_list(
      ctx.native_handle(), ciphers.c_str());

  ctx.use_certificate(
    asio::const_buffer(crt.data(), crt.length()), asio::ssl::context::pem);

  ctx.use_rsa_private_key(
    asio::const_buffer(key.data(), key.length()), asio::ssl::context::pem);
  
  if(ca.empty()) {
    ctx.set_default_verify_paths();
  } else {
    ctx.add_certificate_authority(
      asio::const_buffer(ca.data(), ca.length()));
  }

  SSL_CTX_set_ecdh_auto(ctx.native_handle(), 1);
  //SSL_CTX_set_tmp_dh(
  //  ctx.native_handle(), EC_KEY_new_by_curve_name (NID_X9_62_prime256v1));
  //ctx.use_tmp_dh_file("dh2048.pem");
  
  //ctx.set_verify_mode(asio::ssl::verify_peer);
}

SWC_SHOULD_INLINE
void ConfigSSL::make_server(AppContext::Ptr& app_ctx, SocketPlain& socket) {
  asio::ssl::context ssl_ctx(asio::ssl::context::tlsv13_server);
  configure_server(ssl_ctx);
      
  auto conn = std::make_shared<ConnHandlerSSL>(app_ctx, ssl_ctx, socket);
  conn->new_connection();
  conn->handshake();
}


void ConfigSSL::configure_client(asio::ssl::context& ctx) const {
  ctx.set_options(
      asio::ssl::context::no_compression
    | asio::ssl::context::no_sslv2
    | asio::ssl::context::no_sslv3
    | asio::ssl::context::no_tlsv1
    | asio::ssl::context::no_tlsv1_1
    | asio::ssl::context::no_tlsv1_2
  );

  if(!ciphers.empty())
    SSL_CTX_set_cipher_list(
      ctx.native_handle(), ciphers.c_str());

  if(ca.empty()) {
    ctx.set_default_verify_paths();
  } else {
    ctx.add_certificate_authority(
      asio::const_buffer(ca.data(), ca.length()));
  }

  //ctx.set_verify_mode(asio::ssl::verify_peer);
}

std::shared_ptr<ConnHandlerSSL> ConfigSSL::make_client(
      AppContext::Ptr& app_ctx, SocketPlain& socket, asio::error_code& ec) {
  asio::ssl::context ssl_ctx(asio::ssl::context::tlsv13_client);
  configure_client(ssl_ctx);
  auto conn = std::make_shared<ConnHandlerSSL>(app_ctx, ssl_ctx, socket);
  conn->new_connection();
  
  if(!subject_name.empty())
    conn->set_verify(
      [this, conn](bool preverified, asio::ssl::verify_context& ctx) { 
        return verify(preverified, ctx); 
    });

  conn->handshake_client(ec);

  return conn;
}

void ConfigSSL::make_client(AppContext::Ptr& app_ctx, SocketPlain& socket,
                            const HandshakeCb_t& cb) { 
  asio::ssl::context ssl_ctx(asio::ssl::context::tlsv13_client);
  configure_client(ssl_ctx);
  auto conn = std::make_shared<ConnHandlerSSL>(app_ctx, ssl_ctx, socket);
  conn->new_connection();

  if(!subject_name.empty())
    conn->set_verify(
      [this, conn](bool preverified, asio::ssl::verify_context& ctx) { 
        return verify(preverified, ctx); 
    });

  conn->handshake_client(
    [conn, cb](const asio::error_code& ec) { cb(conn, ec); } );
}

bool ConfigSSL::verify(bool preverified, asio::ssl::verify_context& ctx) {
  //OR return asio::ssl::rfc2818_verification(name);
  if(preverified)
    return false;
  char name[256];
  X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
  X509_NAME_oneline(X509_get_subject_name(cert), name, 256);
  preverified = strcmp(((const char*)name)+4, subject_name.c_str()) == 0;
  SWC_LOGF(LOG_DEBUG, "verify state=%d crt(%s)==%s ", 
            preverified, ((const char*)name)+4, subject_name.c_str());
  return preverified; 
}

void ConfigSSL::load_file(std::string filepath, std::string& to) const {
  if(filepath.front() != '.' && filepath.front() != '/')
    filepath = Env::Config::settings()->get_str("swc.cfg.path") + filepath;

  to.clear();
  errno = 0;
  FileUtils::read(filepath, to);
  if(errno) {
    SWC_THROWF(Error::CONFIG_BAD_VALUE, "Bad File '%s' error=%d(%s)",
              filepath.c_str(), errno, Error::get_text(errno));
  }
}

}
