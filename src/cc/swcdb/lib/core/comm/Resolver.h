/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_comm_Resolver_h
#define swc_core_comm_Resolver_h

#include "../Serialization.h"
#include <string>
#include <vector>

namespace SWC { 

typedef asio::ip::tcp::endpoint EndPoint;
typedef std::vector<EndPoint> EndPoints;

namespace Serialization {
  
inline size_t encoded_length(const EndPoint& endpoint){
  return 3 + (endpoint.address().is_v4() ? 4 : 16);
}

inline void encode(const EndPoint& endpoint, uint8_t **bufp){
  Serialization::encode_bool(bufp, endpoint.address().is_v4());
  Serialization::encode_i16(bufp, endpoint.port());
  if(endpoint.address().is_v4()) {
    Serialization::encode_i32(bufp, endpoint.address().to_v4().to_ulong());
    return;
  }
    
  auto v6_bytes = endpoint.address().to_v6().to_bytes();
  Serialization::encode_bytes(bufp, v6_bytes.data(), 16);
}
  
inline EndPoint decode(const uint8_t **bufp, size_t *remainp){
  bool is_v4 = Serialization::decode_bool(bufp, remainp);
  uint32_t port = Serialization::decode_i16(bufp, remainp);
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


inline bool has_endpoint(const EndPoint& e1, const EndPoints& endpoints_in){
  if(endpoints_in.size()==0) 
    return false;
  return std::find_if(endpoints_in.begin(), endpoints_in.end(),  
          [e1](const EndPoint& e2){
            return e1.address() == e2.address() && e1.port() == e2.port();}) 
        != endpoints_in.end();
}

inline bool has_endpoint(const EndPoints& endpoints, const EndPoints& endpoints_in){
  for(auto& endpoint : endpoints){
    if(has_endpoint(endpoint, endpoints_in)) return true;
  }
  return false;
}


inline size_t endpoints_hash(const EndPoints& endpoints){
  std::string s;
  for(auto& endpoint : endpoints){
    s.append(endpoint.address().to_string());
    s.append(":");
    s.append(std::to_string(endpoint.port()));
  }
  std::hash<std::string> hasher;
  return hasher(s);
}

inline size_t endpoint_hash(const EndPoint& endpoint){
  std::hash<std::string> hasher;
  return hasher(
    (std::string)endpoint.address().to_string()
    +":"
    +std::to_string(endpoint.port()));
}



namespace Resolver {

inline bool is_ipv4_address(const std::string& str)
{
  struct sockaddr_in sa;
  return inet_pton(AF_INET, str.c_str(), &(sa.sin_addr))!=0;
}

inline bool is_ipv6_address(const std::string& str)
{
  struct sockaddr_in6 sa;
  return inet_pton(AF_INET6, str.c_str(), &(sa.sin6_addr))!=0;
}

inline EndPoints get_endpoints(int32_t defaul_port, 
                        Strings &addrs, 
                        std::string &host, 
                        bool srv=false){

  EndPoints endpoints;
  if(!addrs.empty()) {
    std::string ip;
    uint32_t port;
    for(auto& addr : addrs) {
      auto at = addr.find_first_of("-");
      if(at != std::string::npos) {
        ip = addr.substr(0, at);  
        port = SWC::Property::int32_t_from_string(addr.substr(at+1));
      } else {
        ip = addr;  
        port = defaul_port;
      }
      endpoints.push_back(
        asio::ip::tcp::endpoint(
          asio::ip::make_address(ip.c_str()), 
          (uint32_t)port
      ));
    }

  } else if(!host.empty()) {
    uint32_t port;
    std::string ip;
    std::string hostname;
    auto hostname_cfg = host;
    auto at = host.find_first_of(":");
    if(at != std::string::npos) {
        hostname = host.substr(0, at);  
        port = SWC::Property::int32_t_from_string(host.substr(at+1));
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
      HT_THROWF(Error::COMM_SOCKET_ERROR, 
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
        HT_THROWF(Error::COMM_SOCKET_ERROR, 
                  "Bad IP for host: %s, address-info: ", 
                  hostname.c_str(), rp->ai_addr);
        
      endpoints.push_back(
        asio::ip::tcp::endpoint(asio::ip::make_address(c_addr), (uint32_t)port
      ));
    }
  }
  
  if(srv && endpoints.empty()){
    endpoints.push_back(
      asio::ip::tcp::endpoint(asio::ip::make_address("::"), defaul_port
    ));
  }
  return endpoints;
}



}}

#endif // swc_core_comm_Resolver_h