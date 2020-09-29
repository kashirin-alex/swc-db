/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_core_comm_Resolver_h
#define swc_core_comm_Resolver_h

#include <asio.hpp>
#include <string>
#include <vector>
#include "swcdb/core/config/Property.h"

namespace SWC {

namespace Comm { 

typedef asio::ip::tcp::endpoint EndPoint;
typedef std::vector<EndPoint> EndPoints;

struct Network {
  bool                        is_v4;
  const asio::ip::network_v4  v4;
  const asio::ip::network_v6  v6;

  Network(const asio::ip::network_v4& v4);
  Network(const asio::ip::network_v6& v6);
  Network(const Network& net);
};

} //namespace Comm



namespace Serialization {
  
size_t encoded_length(const Comm::EndPoint& endpoint);

void encode(const Comm::EndPoint& endpoint, uint8_t** bufp);

Comm::EndPoint decode(const uint8_t** bufp, size_t* remainp);

} //namespace Serialization



namespace Comm {

bool has_endpoint(const EndPoint& e1, const EndPoints& endpoints_in);

bool has_endpoint(const EndPoints& endpoints, const EndPoints& endpoints_in);


size_t endpoints_hash(const EndPoints& endpoints);

size_t endpoint_hash(const EndPoint& endpoint);


namespace Resolver {

bool is_ipv4_address(const std::string& str);

bool is_ipv6_address(const std::string& str);

EndPoints get_endpoints(uint16_t defaul_port, 
                        const Config::Strings& addrs, 
                        const std::string& host, 
                        const std::vector<Network>& nets,
                        bool srv=false);

void sort(const std::vector<Network>& nets, const EndPoints& endpoints, 
          EndPoints& sorted);

void get_networks(const Config::Strings& networks, 
                  std::vector<Network>& nets, asio::error_code& ec);

void get_networks(const Config::Strings& networks, 
                  std::vector<asio::ip::network_v4>& nets_v4, 
                  std::vector<asio::ip::network_v6>& nets_v6,
                  asio::error_code& ec);

bool is_network(const EndPoint& endpoint,
                const std::vector<asio::ip::network_v4>& nets_v4, 
                const std::vector<asio::ip::network_v6>& nets_v6);

bool is_network(const EndPoint& endpoint, const asio::ip::network_v4& net);

bool is_network(const EndPoint& endpoint, const asio::ip::network_v6& net);


}} // namespace :Comm::Resolver


} // namespace SWC



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/Resolver.cc"
#endif 

#endif // swc_core_comm_Resolver_h