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


// Send a checksum-valid header that claims a payload larger than
// swc.comm.payload.max, and verify the server closes without waiting to
// read the body (R-P0-2 unbounded-alloc guard).
static bool expect_close_on_oversized_payload(const Comm::EndPoint& endpoint,
                                              uint32_t payload_size) {
  asio::io_context ioc(1);
  asio::ip::tcp::socket sock(ioc);
  asio::error_code ec;

  sock.open(endpoint.protocol(), ec);
  SWC_ASSERT(!ec);
  sock.connect(endpoint, ec);
  SWC_ASSERT(!ec);

  Comm::Header hdr;
  hdr.flags = Comm::Header::FLAG_REQUEST_BIT;
  hdr.id = 1;
  hdr.timeout_ms = 10000;
  hdr.command = 0;
  hdr.data.size = payload_size;
  hdr.encoded_length();

  uint8_t buf[Comm::Header::MAX_LENGTH];
  uint8_t* ptr = buf;
  hdr.encode(&ptr);
  SWC_ASSERT(size_t(ptr - buf) == hdr.header_len);

  asio::write(sock, asio::buffer(buf, hdr.header_len), ec);
  SWC_ASSERT(!ec);

  bool closed = false;
  uint8_t rbuf[64];

  asio::steady_timer timer(ioc);
  timer.expires_after(std::chrono::seconds(5));
  timer.async_wait([&sock](const asio::error_code& e) {
    if(!e) {
      asio::error_code ecc;
      sock.cancel(ecc);
    }
  });

  sock.async_read_some(
    asio::buffer(rbuf, sizeof(rbuf)),
    [&closed, &timer](const asio::error_code& e, size_t) {
      // eof / connection-reset => server closed (expected for oversize).
      // operation_aborted => deadline; server still waiting to read body.
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

  uint64_t max_payload = Env::Config::settings()
    ->get<Config::Property::Value_uint64_g>("swc.comm.payload.max")->get();
  // Zero disables the check; this test needs a finite limit.
  SWC_ASSERT(max_payload);
  SWC_ASSERT(max_payload < UINT32_MAX);

  // Claim one byte over the configured max. Fixed servers reject before
  // allocate; vulnerable ones wait for the body — close-vs-wait distinguishes.
  uint32_t oversize = uint32_t(max_payload + 1);

  int failures = 0;
  if(!expect_close_on_oversized_payload(endpoint, oversize)) {
    SWC_LOG(LOG_ERROR,
      "server did NOT close on oversized payload (alloc guard missing)");
    ++failures;
  }

  Env::Clients::get()->stop();

  SWC_ASSERT(!failures);
  return 0;
}
