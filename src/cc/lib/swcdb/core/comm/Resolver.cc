/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/comm/Resolver.h"
#include "swcdb/core/Serialization.h"

namespace SWC { 

namespace Serialization {
  
size_t encoded_length(const EndPoint& endpoint) {
  return 3 + (endpoint.address().is_v4() ? 4 : 16);
}

void encode(const EndPoint& endpoint, uint8_t** bufp) {
  Serialization::encode_bool(bufp, endpoint.address().is_v4());
  Serialization::encode_i16(bufp, endpoint.port());
  if(endpoint.address().is_v4()) {
    Serialization::encode_i32(bufp, endpoint.address().to_v4().to_ulong());
    return;
  }
    
  auto v6_bytes = endpoint.address().to_v6().to_bytes();
  Serialization::encode_bytes(bufp, v6_bytes.data(), 16);
}
  
EndPoint decode(const uint8_t** bufp, size_t* remainp) {
  bool is_v4 = Serialization::decode_bool(bufp, remainp);
  uint16_t port = Serialization::decode_i16(bufp, remainp);
  if(is_v4) 
    return EndPoint(
      asio::ip::make_address_v4(
        Serialization::decode_i32(bufp, remainp)), 
        port);
  
  uint8_t* bytes = Serialization::decode_bytes(bufp, remainp, 16);
  asio::ip::address_v6::bytes_type v6_bytes;
  std::memcpy(v6_bytes.data(), bytes, 16);

  return EndPoint(asio::ip::address_v6(v6_bytes), port);
}

}


bool has_endpoint(const EndPoint& e1, const EndPoints& endpoints_in) {
  if(endpoints_in.size()==0) 
    return false;
  return std::find_if(endpoints_in.begin(), endpoints_in.end(),  
          [e1](const EndPoint& e2){
            return e1.address() == e2.address() && e1.port() == e2.port();}) 
        != endpoints_in.end();
}

bool has_endpoint(const EndPoints& endpoints, 
                        const EndPoints& endpoints_in) {
  for(auto& endpoint : endpoints){
    if(has_endpoint(endpoint, endpoints_in)) return true;
  }
  return false;
}


size_t endpoints_hash(const EndPoints& endpoints) {
  std::string s;
  for(auto& endpoint : endpoints){
    s.append(endpoint.address().to_string());
    s.append(":");
    s.append(std::to_string(endpoint.port()));
  }
  std::hash<std::string> hasher;
  return hasher(s);
}

size_t endpoint_hash(const EndPoint& endpoint) {
  std::hash<std::string> hasher;
  return hasher(
    (std::string)endpoint.address().to_string()
    +":"
    +std::to_string(endpoint.port()));
}



namespace Resolver {

SWC_SHOULD_INLINE
bool is_ipv4_address(const std::string& str) {
  struct sockaddr_in sa;
  return inet_pton(AF_INET, str.c_str(), &(sa.sin_addr)) != 0;
}

SWC_SHOULD_INLINE
bool is_ipv6_address(const std::string& str) {
  struct sockaddr_in6 sa;
  return inet_pton(AF_INET6, str.c_str(), &(sa.sin6_addr)) != 0;
}

EndPoints get_endpoints(uint16_t defaul_port, 
                        const Strings& addrs, 
                        const std::string& host, 
                        const std::vector<Network>& nets,
                        bool srv) {
  EndPoints endpoints;
  std::string ip;
  uint16_t port;
  if(!addrs.empty()) {
    for(auto& addr : addrs) {
      auto at = addr.find_first_of("-");
      if(at != std::string::npos) {
        ip = addr.substr(0, at);  
        Property::from_string(addr.substr(at+1), &port);
      } else {
        ip = addr;  
        port = defaul_port;
      }
      endpoints.emplace_back(asio::ip::make_address(ip.c_str()), port );
    }

  } else if(!host.empty()) {
    std::string hostname;
    auto hostname_cfg = host;
    auto at = host.find_first_of(":");
    if(at != std::string::npos) {
        hostname = host.substr(0, at);  
        Property::from_string(host.substr(at+1), &port);
      } else {
        port = defaul_port;
        hostname = host;  
      }
    addrinfo hints, *result, *rp;
    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_UNSPEC; 
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_protocol = 0;    
    hints.ai_flags = 0; //AI_CANONNAME | AI_ALL | AI_ADDRCONFIG;

    int x =  getaddrinfo(hostname.c_str(), NULL, &hints, &result);
    if(x != 0){
      SWC_THROWF(Error::COMM_SOCKET_ERROR, 
                "Bad addr-info for host: %s", hostname.c_str());
    }
      
    for (rp = result; rp != NULL; rp = rp->ai_next) {
      char c_addr[INET6_ADDRSTRLEN];
      const char * s;
      switch(rp->ai_family){
        case AF_INET: {
          s = inet_ntop(AF_INET,
            &(((struct sockaddr_in *)rp->ai_addr)->sin_addr),  
            c_addr, INET_ADDRSTRLEN);
          break;
        }
        case AF_INET6:{
          s = inet_ntop(AF_INET6, 
            &(((struct sockaddr_in6  *)rp->ai_addr)->sin6_addr), 
            c_addr, INET6_ADDRSTRLEN);
          break;
        }
       default:
         break;
      }
      if(s == NULL) 
        SWC_THROWF(Error::COMM_SOCKET_ERROR, 
                  "Bad IP for host: %s, address-info: ", 
                  hostname.c_str(), rp->ai_addr);
        
      endpoints.emplace_back(asio::ip::make_address(c_addr), port);
    }
  }
  
  if(srv && endpoints.empty()) {
    endpoints.emplace_back(asio::ip::make_address("::"), defaul_port);
    return endpoints;
  }
  
  if(nets.empty())
    return endpoints;
  
  EndPoints sorted;
  sort(nets, endpoints, sorted);
  return sorted;
}

void sort(const std::vector<Network>& nets, const EndPoints& endpoints, 
          EndPoints& sorted) {
  // sort endpoints by Network Priority Access
  asio::error_code ec;

  for(auto& net : nets) {
    for(auto& endpoint : endpoints) {
      if((net.is_v4 && endpoint.address().is_v4() && 
          is_network(endpoint, net.v4)) ||
         (!net.is_v4 && endpoint.address().is_v6() && 
          is_network(endpoint, net.v6)))
        sorted.push_back(endpoint);
    }
  }
  
  for(auto& endpoint : endpoints) { // add rest
    if(!has_endpoint(endpoint, sorted))
      sorted.push_back(endpoint);
  }
}

void get_networks(const Strings& networks, 
                  std::vector<Network>& nets, asio::error_code& ec) {
  for(auto& net : networks) {
    Network n;
    if(n.is_v4 = net.find_first_of(":") == std::string::npos)
      n.v4 = asio::ip::make_network_v4(net, ec);
    else
      n.v6 = asio::ip::make_network_v6(net, ec);
    nets.push_back(n);
  }
}

void get_networks(const Strings& networks, 
                  std::vector<asio::ip::network_v4>& nets_v4, 
                  std::vector<asio::ip::network_v6>& nets_v6,
                  asio::error_code& ec) {
  for(auto& net : networks) {
    if(net.find_first_of(":") == std::string::npos)
      nets_v4.push_back(asio::ip::make_network_v4(net, ec));      
    else
      nets_v6.push_back(asio::ip::make_network_v6(net, ec));
  }
}

bool is_network(const EndPoint& endpoint,
                const std::vector<asio::ip::network_v4>& nets_v4, 
                const std::vector<asio::ip::network_v6>& nets_v6) {
  if(endpoint.address().is_v4()) {
    for(auto& net : nets_v4)
      if(is_network(endpoint, net))
        return true;
    return false;
  }
  if(endpoint.address().is_v6()) {
    for(auto& net : nets_v6) {
      if(is_network(endpoint, net))
        return true;
    }
    return false;
  }
  return false;
}

bool is_network(const EndPoint& endpoint, const asio::ip::network_v4& net) {
  return endpoint.address().to_v4() == net.address() || 
    asio::ip::make_network_v4(endpoint.address().to_v4(), 32)
      .is_subnet_of(net);
}

bool is_network(const EndPoint& endpoint, const asio::ip::network_v6& net) {    
  return endpoint.address().to_v6() == net.address() || 
    asio::ip::make_network_v6(endpoint.address().to_v6(), 128)
      .is_subnet_of(net);
}

}}