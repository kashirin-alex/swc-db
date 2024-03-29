/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/comm/ConfigSSL.h"
#include "swcdb/core/comm/Settings.h"
#include <fstream>


namespace SWC { namespace Comm {

SWC_SHOULD_NOT_INLINE
ConfigSSL::ConfigSSL(const Config::Settings& settings, bool is_client)
                    : nets_v4(), nets_v6(), subject_name(),
                      ctx(is_client
                            ? asio::ssl::context::tlsv13_client
                            : asio::ssl::context::tlsv13_server) {
  set_networks(settings.get_strs("swc.comm.ssl.secure.network"), is_client);

  std::string ciphers = settings.get_str("swc.comm.ssl.ciphers", "");
  if(!ciphers.empty())
    SSL_CTX_set_cipher_list(
      ctx.native_handle(), ciphers.c_str());

  const std::string pathbase(settings.get_str("swc.cfg.path"));
  std::string ca;
  if(settings.has("swc.comm.ssl.ca"))
    load_file(pathbase, settings.get_str("swc.comm.ssl.ca"), ca);
  if(ca.empty()) {
    ctx.set_default_verify_paths();
  } else {
    ctx.add_certificate_authority(
      asio::const_buffer(ca.data(), ca.length()));
  }


  if(is_client) {
    ctx.set_options(
        asio::ssl::context::no_compression
      | asio::ssl::context::no_sslv2
      | asio::ssl::context::no_sslv3
      | asio::ssl::context::no_tlsv1
      | asio::ssl::context::no_tlsv1_1
      | asio::ssl::context::no_tlsv1_2
    );
    subject_name = settings.get_str("swc.comm.ssl.subject_name", "");
    if(!subject_name.empty())
      ctx.set_verify_mode(asio::ssl::verify_peer);

  } else {
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
    );

    std::string  crt;
    load_file(pathbase, settings.get_str("swc.comm.ssl.crt"), crt);
    ctx.use_certificate(
      asio::const_buffer(crt.data(), crt.length()), asio::ssl::context::pem);

    std::string  key;
    load_file(pathbase, settings.get_str("swc.comm.ssl.key"), key);
    ctx.use_rsa_private_key(
      asio::const_buffer(key.data(), key.length()), asio::ssl::context::pem);
  }

  /*
  SSL_CTX_set_ecdh_auto(ctx.native_handle(), 1);
  SSL_CTX_set_tmp_dh(
    ctx.native_handle(), EC_KEY_new_by_curve_name (NID_X9_62_prime256v1));
  ctx.use_tmp_dh_file("dh2048.pem");
  ctx.set_verify_mode(
    asio::ssl::verify_client_once | asio::ssl::verify_fail_if_no_peer_cert);
  */
}


void ConfigSSL::set_networks(const Config::Strings& networks,
                             bool with_local) {
  // option. tmp nets-config
  asio::error_code ec;
  Resolver::get_networks(networks, nets_v4, nets_v6, ec);
  if(ec)
    SWC_THROWF(Error::CONFIG_BAD_VALUE,
              "Bad Network in swc.comm.ssl.secure.network error(%s)",
              ec.message().c_str());
  if(!with_local) // option pass
    return;

  // option tmp nets-local for srv by the binded endpoints
  int err = Error::OK;
  Resolver::get_local_networks(err, nets_v4, nets_v6);
  if(err)
    SWC_THROWF(Error::CONFIG_BAD_VALUE,
              "Bad Network in swc.comm.ssl.secure.network error(%s)",
              Error::get_text(err));
  /* option
    includes and excludes subnets/racks in secure-networks
    clear nets-wide not in nets-local
    mv nets-wide to nets_v4, nets_v6
  */
}


ConnHandlerSSL::Ptr
ConfigSSL::make_client(AppContext::Ptr& app_ctx,
                       SocketPlain& socket) const {
  auto conn = make_connection(app_ctx, socket);
  if(!subject_name.empty())
    conn->set_verify(asio::ssl::host_name_verification(subject_name));
  return conn;
}

ConnHandlerPtr
ConfigSSL::make_client(AppContext::Ptr& app_ctx,
                       SocketPlain& socket,
                       asio::error_code& ec) const {
  auto conn = make_client(app_ctx, socket);
  conn->handshake(SocketSSL::client, ec);
  return conn;
}

void ConfigSSL::load_file(const std::string& pathbase,
                          std::string filepath, std::string& to) const {
  to.clear();
  int err = Error::OK;
  try {
    if(filepath.front() != '.' && filepath.front() != '/')
      filepath = pathbase + filepath;

    std::ifstream istrm(filepath, std::ios::binary | std::ios::ate);
    if(istrm.is_open()) {
      if(size_t len = istrm.tellg()) {
        to.resize(len);
        istrm.seekg(0);
        istrm.read(to.data(), to.length());
      }
      istrm.close();
    } else {
      err = Error::CONFIG_BAD_CFG_FILE;
    }
  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    err = e.code();
  }
  if(err) {
    SWC_THROWF(Error::CONFIG_BAD_VALUE, "Bad File '%s' error=%d(%s)",
              filepath.c_str(), err, Error::get_text(err));
  }
}


}}
