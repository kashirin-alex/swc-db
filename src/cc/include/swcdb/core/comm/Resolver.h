/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_comm_Resolver_h
#define swc_core_comm_Resolver_h

#include <asio.hpp>
#include <string>
#include <vector>
#include "swcdb/core/config/Property.h"

namespace SWC { 

typedef asio::ip::tcp::endpoint EndPoint;
typedef std::vector<EndPoint> EndPoints;

namespace Serialization {
  
const size_t encoded_length(const EndPoint& endpoint);

void encode(const EndPoint& endpoint, uint8_t **bufp);

EndPoint decode(const uint8_t **bufp, size_t *remainp);

}


const bool has_endpoint(const EndPoint& e1, 
                        const EndPoints& endpoints_in);

const bool has_endpoint(const EndPoints& endpoints, 
                        const EndPoints& endpoints_in);


const size_t endpoints_hash(const EndPoints& endpoints);

const size_t endpoint_hash(const EndPoint& endpoint);



namespace Resolver {

const bool is_ipv4_address(const std::string& str);

const bool is_ipv6_address(const std::string& str);

EndPoints get_endpoints(uint16_t defaul_port, 
                        const Strings &addrs, 
                        const std::string &host, 
                        bool srv=false);

}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/Resolver.cc"
#endif 

#endif // swc_core_comm_Resolver_h