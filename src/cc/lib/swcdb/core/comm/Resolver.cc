/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/comm/Resolver.h"
#include "swcdb/core/Serialization.h"

namespace SWC { 


namespace Serialization {
  
size_t encoded_length(const Comm::EndPoint& endpoint) {
  return 3 + (endpoint.address().is_v4() ? 4 : 16);
}

void encode(const Comm::EndPoint& endpoint, uint8_t** bufp) {
  Serialization::encode_bool(bufp, endpoint.address().is_v4());
  Serialization::encode_i16(bufp, endpoint.port());
  if(endpoint.address().is_v4()) {
    auto v4_bytes = endpoint.address().to_v4().to_bytes();
    Serialization::encode_bytes_fixed(bufp, v4_bytes.data(), v4_bytes.size());

  } else { 
    auto v6_bytes = endpoint.address().to_v6().to_bytes();
    Serialization::encode_bytes_fixed(bufp, v6_bytes.data(), v6_bytes.size());
  }
}
  
Comm::EndPoint decode(const uint8_t** bufp, size_t* remainp) {
  bool is_v4 = Serialization::decode_bool(bufp, remainp);
  uint16_t port = Serialization::decode_i16(bufp, remainp);
  if(is_v4) {
    asio::ip::address_v4::bytes_type v4_bytes;
    const uint8_t* bytes = Serialization::decode_bytes_fixed(
      bufp, remainp, v4_bytes.size());
    std::memcpy(v4_bytes.data(), bytes, v4_bytes.size());
    return Comm::EndPoint(asio::ip::make_address_v4(v4_bytes), port);
  }
  
  asio::ip::address_v6::bytes_type v6_bytes;
    const uint8_t* bytes = Serialization::decode_bytes_fixed(
      bufp, remainp, v6_bytes.size());
  std::memcpy(v6_bytes.data(), bytes, v6_bytes.size());
  return Comm::EndPoint(asio::ip::address_v6(v6_bytes), port);
}


} // namespace Serialization



namespace Comm {


Network::Network(const asio::ip::network_v4& v4)
  : is_v4(true), v4(v4) {
}

Network::Network(const asio::ip::network_v6& v6)
  : is_v4(false), v6(v6) {
}

Network::Network(const Network& net)
  : is_v4(net.is_v4), v4(net.v4), v6(net.v6) {      
}


bool has_endpoint(const EndPoint& e1, const EndPoints& endpoints_in) {
  return !endpoints_in.empty() &&
         std::find(endpoints_in.begin(), endpoints_in.end(), e1)
            != endpoints_in.end();
}

bool has_endpoint(const EndPoints& endpoints, 
                  const EndPoints& endpoints_in) {
  for(auto& endpoint : endpoints) {
    if(has_endpoint(endpoint, endpoints_in)) 
      return true;
  }
  return false;
}


size_t endpoints_hash(const EndPoints& endpoints) {
  std::string s;
  for(auto& endpoint : endpoints) {
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
    endpoint.address().to_string() + "_" + std::to_string(endpoint.port()));
}




SWC_SHOULD_INLINE
bool Resolver::is_ipv4_address(const std::string& str) {
  struct sockaddr_in sa;
  return inet_pton(AF_INET, str.c_str(), &(sa.sin_addr));
}

SWC_SHOULD_INLINE
bool Resolver::is_ipv6_address(const std::string& str) {
  struct sockaddr_in6 sa;
  return inet_pton(AF_INET6, str.c_str(), &(sa.sin6_addr));
}

EndPoints Resolver::get_endpoints(uint16_t defaul_port, 
                                  const Config::Strings& addrs,
                                  const std::string& host,
                                  const Networks& nets,
                                  bool srv) {
  EndPoints endpoints;
  std::string ip;
  uint16_t port;
  if(!addrs.empty()) {
    for(auto& addr : addrs) {
      auto at = addr.find_first_of("-");
      if(at != std::string::npos) {
        ip = addr.substr(0, at);  
        Config::Property::from_string(addr.substr(at+1), &port);
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
      Config::Property::from_string(host.substr(at+1), &port);
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

    errno = 0;
    int x =  getaddrinfo(hostname.c_str(), NULL, &hints, &result);
    if(x) {
      x = x == EAI_SYSTEM ? errno : (SWC_ERRNO_EAI_BEGIN + (-x));
      SWC_THROWF(x, "Bad addr-info for host: %s", hostname.c_str());
    }
      
    for (rp = result; rp != NULL; rp = rp->ai_next) {
      char c_addr[INET6_ADDRSTRLEN];
      const char * s = nullptr;
      switch(rp->ai_family) {
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
      if(!s) {
        x = SWC_ERRNO_EAI_BEGIN + (-EAFNOSUPPORT);
        SWC_THROWF(x, "Bad IP for host: %s", hostname.c_str());
      }
        
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

void Resolver::sort(const Networks& nets, const EndPoints& endpoints, 
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

void Resolver::get_networks(const Config::Strings& networks, 
                            Networks& nets, asio::error_code& ec) {
  nets.reserve(networks.size());
  for(auto& net : networks) {
    if(net.find_first_of(":") == std::string::npos)
      nets.emplace_back(asio::ip::make_network_v4(net, ec));
    else
      nets.emplace_back(asio::ip::make_network_v6(net, ec));
  }
}

void Resolver::get_networks(const Config::Strings& networks, 
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

void Resolver::get_local_networks(int& err, 
                                  std::vector<asio::ip::network_v4>& nets_v4,
                                  std::vector<asio::ip::network_v6>& nets_v6)
                                  noexcept {
  errno = 0; 
  char hostname[256];
  err = gethostname(hostname, sizeof(hostname));
  if(err == -1) {
    err = errno;
    return;
  }
  
  addrinfo hints, *result, *rp;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC; 
  hints.ai_socktype = SOCK_STREAM; 
  hints.ai_protocol = 0;    
  hints.ai_flags = 0;

  errno = 0; 
  if((err = getaddrinfo(hostname, NULL, &hints, &result))) {
    err = err == EAI_SYSTEM ? errno : (SWC_ERRNO_EAI_BEGIN + (-err));
    return;
  }
  
  asio::error_code ec;
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    char c_addr[INET6_ADDRSTRLEN];
    const char * s = nullptr;
    errno = 0; 
    switch(rp->ai_family) {
      case AF_INET: {
        s = inet_ntop(AF_INET,
          &(((struct sockaddr_in *)rp->ai_addr)->sin_addr),  
          c_addr, INET_ADDRSTRLEN);
        if(s)
          nets_v4.push_back(
            asio::ip::make_network_v4(
              asio::ip::make_address_v4(c_addr, ec), 32));
        break;
      }
      case AF_INET6: {
        s = inet_ntop(AF_INET6, 
          &(((struct sockaddr_in6  *)rp->ai_addr)->sin6_addr), 
          c_addr, INET6_ADDRSTRLEN);
        if(s)
          nets_v6.push_back(
            asio::ip::make_network_v6(
              asio::ip::make_address_v6(c_addr, ec), 128));
        break;
      }
     default:
       break;
    }
    if(!s) {
      err = SWC_ERRNO_EAI_BEGIN + (-EAFNOSUPPORT);
      return;
    }
    if(ec) {
      err = ec.value();
      return;
    }
    if(errno) {
      err = errno;
      return;
    }
  }
}

bool Resolver::is_network(const EndPoint& endpoint,
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

bool Resolver::is_network(const EndPoint& endpoint, 
                          const asio::ip::network_v4& net) {
  return endpoint.address().to_v4() == net.address() || 
    asio::ip::make_network_v4(endpoint.address().to_v4(), 32)
      .is_subnet_of(net);
}

bool Resolver::is_network(const EndPoint& endpoint, 
                          const asio::ip::network_v6& net) {    
  return endpoint.address().to_v6() == net.address() || 
    asio::ip::make_network_v6(endpoint.address().to_v6(), 128)
      .is_subnet_of(net);
}

}} // namespace SWC::Comm
