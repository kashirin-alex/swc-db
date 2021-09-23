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
typedef Core::Vector<EndPoint>  EndPoints;


SWC_CAN_INLINE
std::ostream& operator<<(std::ostream& out, const EndPoint& endpoint) {
  return (
    endpoint.address().is_v4()
      ? (out << endpoint.address().to_v4().to_string())
      : (out << '[' << endpoint.address().to_v6().to_string() << ']')
    ) << ':' << endpoint.port();
}


struct Network {
  bool                        is_v4;
  const asio::ip::network_v4  v4;
  const asio::ip::network_v6  v6;

  SWC_CAN_INLINE
  Network(const asio::ip::network_v4& a_v4) noexcept
          : is_v4(true), v4(a_v4) { }

  SWC_CAN_INLINE
  Network(const asio::ip::network_v6& a_v6) noexcept
          : is_v4(false), v6(a_v6) { }

  SWC_CAN_INLINE
  Network(const Network& net) noexcept
          : is_v4(net.is_v4), v4(net.v4), v6(net.v6) {
  }
};

typedef Core::Vector<Network>              Networks;
typedef Core::Vector<asio::ip::network_v4> Networks_v4;
typedef Core::Vector<asio::ip::network_v6> Networks_v6;

} //namespace Comm



namespace Serialization {

SWC_CAN_INLINE
uint8_t encoded_length(const Comm::EndPoint& endpoint) noexcept {
  return 3 + (endpoint.address().is_v4() ? 4 : 16);
}

void encode(uint8_t** bufp, const Comm::EndPoint& endpoint);

Comm::EndPoint decode(const uint8_t** bufp, size_t* remainp);


uint32_t SWC_PURE_FUNC
encoded_length(const Comm::EndPoints& endpoints) noexcept;

void encode(uint8_t** bufp, const Comm::EndPoints& endpoints);

void decode(const uint8_t** bufp, size_t* remainp,
            Comm::EndPoints& endpoints);

SWC_CAN_INLINE
Comm::EndPoints decode_endpoints(const uint8_t** bufp, size_t* remainp) {
  Comm::EndPoints endpoints;
  Serialization::decode(bufp, remainp, endpoints);
  return endpoints;
}

} //namespace Serialization



namespace Comm {


void print(std::ostream& out, const EndPoints& endpoints);

bool SWC_PURE_FUNC has_endpoint(const EndPoint& e1,
                                const EndPoints& endpoints_in) noexcept;

bool SWC_PURE_FUNC has_endpoint(const EndPoints& endpoints,
                                const EndPoints& endpoints_in) noexcept;

bool SWC_PURE_FUNC equal_endpoints(const EndPoints& endpoints1,
                                   const EndPoints& endpoints2) noexcept;

size_t endpoints_hash(const EndPoints& endpoints);

size_t SWC_PURE_FUNC endpoint_hash(const EndPoint& endpoint) noexcept;



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
                  Networks_v4& nets_v4,
                  Networks_v6& nets_v6,
                  asio::error_code& ec);

void get_local_networks(int& err,
                        Networks_v4& nets_v4,
                        Networks_v6& nets_v6);

bool is_network(const EndPoint& endpoint,
                const Networks_v4& nets_v4,
                const Networks_v6& nets_v6) noexcept;

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
