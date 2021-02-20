/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_comm_Resolver_h
#define swcdb_core_comm_Resolver_h


#include "swcdb/core/config/Property.h"
#include "swcdb/core/comm/asio_wrap.h"


namespace SWC {

namespace Comm {

typedef asio::ip::tcp::endpoint EndPoint;
typedef std::vector<EndPoint> EndPoints;

struct Network {
  bool                        is_v4;
  const asio::ip::network_v4  v4;
  const asio::ip::network_v6  v6;

  Network(const asio::ip::network_v4& v4) noexcept;
  Network(const asio::ip::network_v6& v6) noexcept;
  Network(const Network& net) noexcept;
};

typedef std::vector<Network> Networks;

} //namespace Comm



namespace Serialization {

uint8_t encoded_length(const Comm::EndPoint& endpoint) noexcept;

void encode(uint8_t** bufp, const Comm::EndPoint& endpoint);

Comm::EndPoint decode(const uint8_t** bufp, size_t* remainp);


uint32_t encoded_length(const Comm::EndPoints& endpoints) noexcept;

void encode(uint8_t** bufp, const Comm::EndPoints& endpoints);

void decode(const uint8_t** bufp, size_t* remainp,
            Comm::EndPoints& endpoints);

} //namespace Serialization



namespace Comm {

bool has_endpoint(const EndPoint& e1,
                  const EndPoints& endpoints_in) noexcept;

bool has_endpoint(const EndPoints& endpoints,
                  const EndPoints& endpoints_in) noexcept;


size_t endpoints_hash(const EndPoints& endpoints);

size_t endpoint_hash(const EndPoint& endpoint);



//! The SWC-DB Resolver C++ namespace 'SWC::Comm::Resolver'
namespace Resolver {


bool is_ipv4_address(const std::string& str) noexcept;

bool is_ipv6_address(const std::string& str) noexcept;

EndPoints get_endpoints(uint16_t defaul_port,
                        const Config::Strings& addrs,
                        const std::string& host,
                        const Networks& nets,
                        bool srv=false);

void sort(const Networks& nets, const EndPoints& endpoints,
          EndPoints& sorted);

void get_networks(const Config::Strings& networks,
                  Networks& nets, asio::error_code& ec);

void get_networks(const Config::Strings& networks,
                  std::vector<asio::ip::network_v4>& nets_v4,
                  std::vector<asio::ip::network_v6>& nets_v6,
                  asio::error_code& ec);

void get_local_networks(int& err,
                        std::vector<asio::ip::network_v4>& nets_v4,
                        std::vector<asio::ip::network_v6>& nets_v6);

bool is_network(const EndPoint& endpoint,
                const std::vector<asio::ip::network_v4>& nets_v4,
                const std::vector<asio::ip::network_v6>& nets_v6) noexcept;

bool is_network(const EndPoint& endpoint,
                const asio::ip::network_v4& net) noexcept;

bool is_network(const EndPoint& endpoint,
                const asio::ip::network_v6& net) noexcept;


}} // namespace Comm::Resolver


} // namespace SWC



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/Resolver.cc"
#endif

#endif // swcdb_core_comm_Resolver_h
