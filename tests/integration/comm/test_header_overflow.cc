/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/Settings.h"
#include "swcdb/core/comm/asio_wrap.h"
#include "swcdb/core/comm/IoContext.h"
#include "swcdb/core/comm/Resolver.h"
#include "swcdb/core/comm/Header.h"

#include "swcdb/db/client/Clients.h"


namespace SWC{ namespace Config {

void init_app_options(Settings* settings) {
  init_comm_options(settings);
  init_client_options(settings);
}

}}

using namespace SWC;


// Send only a crafted 2-byte header prefix carrying `header_len` and verify
// that the server closes the connection instead of reading `header_len`
// bytes into its fixed Header::MAX_LENGTH buffer (R-P0-1 overflow guard).
// A vulnerable server keeps the socket open waiting for the remaining header
// bytes, so a read that only times out (operation_aborted) means "not closed".
static bool expect_close_on_bad_header(const Comm::EndPoint& endpoint,
                                       uint8_t header_len) {
  asio::io_context ioc(1);
  asio::ip::tcp::socket sock(ioc);
  asio::error_code ec;

  sock.open(endpoint.protocol(), ec);
  SWC_ASSERT(!ec);
  sock.connect(endpoint, ec);
  SWC_ASSERT(!ec);

  uint8_t prefix[Comm::Header::PREFIX_LENGTH];
  prefix[0] = Comm::Header::PROTOCOL_VERSION;
  prefix[1] = header_len;
  asio::write(sock, asio::buffer(prefix, Comm::Header::PREFIX_LENGTH), ec);
  SWC_ASSERT(!ec);

  bool closed = false;
  uint8_t rbuf[64];

  asio::steady_timer timer(ioc);
  timer.expires_after(std::chrono::seconds(5));
  timer.async_wait([&sock](const asio::error_code& e) {
    if(!e) {
      asio::error_code ecc;
      sock.cancel(ecc); // deadline reached, abort the pending read
    }
  });

  sock.async_read_some(
    asio::buffer(rbuf, sizeof(rbuf)),
    [&closed, &timer](const asio::error_code& e, size_t) {
      // eof / connection-reset => server closed the connection (expected).
      // operation_aborted => the deadline fired, socket still open (vulnerable).
      // success => server unexpectedly sent data on a bad header.
      if(e && e != asio::error::operation_aborted)
        closed = true;
      timer.cancel();
    }
  );

  ioc.run();
  sock.close(ec);
  return closed;
}


int main(int argc, char** argv) {
  Env::Config::init(argc, argv, &Config::init_app_options, nullptr);

  Env::Clients::init(
    client::Clients::make(
      *Env::Config::settings(),
      Comm::IoContext::make("Clients", 8),
      nullptr, // std::make_shared<client::ManagerContext>()
      nullptr  // std::make_shared<client::RangerContext>()
    )
  );

  Comm::EndPoints endpoints =
    Env::Clients::get()->managers.groups->get_endpoints(
      DB::Types::MngrRole::SCHEMAS, 0, 0);
  SWC_ASSERT(!endpoints.empty());
  const Comm::EndPoint& endpoint = endpoints.front();

  int failures = 0;

  // header_len far beyond the fixed buffer
  if(!expect_close_on_bad_header(endpoint, 255)) {
    SWC_LOG(LOG_ERROR,
      "server did NOT close on header_len=255 (overflow guard missing)");
    ++failures;
  }

  // one byte past the fixed maximum
  if(!expect_close_on_bad_header(
        endpoint, uint8_t(Comm::Header::MAX_LENGTH + 1))) {
    SWC_LOG(LOG_ERROR,
      "server did NOT close on header_len=MAX_LENGTH+1");
    ++failures;
  }

  // header_len == 1 underflows filled = header_len - PREFIX_LENGTH
  if(!expect_close_on_bad_header(endpoint, 1)) {
    SWC_LOG(LOG_ERROR,
      "server did NOT close on header_len=1 (underflow guard missing)");
    ++failures;
  }

  // one byte below the fixed minimum
  if(!expect_close_on_bad_header(
        endpoint, uint8_t(Comm::Header::FIXED_LENGTH - 1))) {
    SWC_LOG(LOG_ERROR,
      "server did NOT close on header_len=FIXED_LENGTH-1");
    ++failures;
  }

  Env::Clients::get()->stop();

  SWC_ASSERT(!failures);
  return 0;
}
