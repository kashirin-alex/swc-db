/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Common/params/HostEndPoints.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace Protocol { namespace Common { namespace Params {
 
HostEndPoints::HostEndPoints() {}

HostEndPoints::HostEndPoints(const EndPoints& points) 
                            : endpoints(points) { }

HostEndPoints::~HostEndPoints() { }

void HostEndPoints::set(const EndPoints& points) {
  endpoints.clear();
  endpoints.assign(points.begin(), points.end());
}

size_t HostEndPoints::internal_encoded_length() const {
  size_t len = Serialization::encoded_length_vi32(endpoints.size());
  for(auto& endpoint : endpoints)
    len += Serialization::encoded_length(endpoint);
  return len;
}

void HostEndPoints::internal_encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, endpoints.size());
  for(auto& endpoint : endpoints)
    Serialization::encode(endpoint, bufp);
}

void HostEndPoints::internal_decode(const uint8_t** bufp, size_t* remainp) {
  size_t len = Serialization::decode_vi32(bufp, remainp);
  endpoints.clear();
  endpoints.resize(len);
  for(size_t i=0;i<len;++i)
    endpoints[i] = Serialization::decode(bufp, remainp);
}

std::string HostEndPoints::to_string() const {
  std::string s("endpoints=(");
  for(auto& endpoint : endpoints){
    s.append("[");
    s.append(endpoint.address().to_string());
    s.append("]:");
    s.append(std::to_string(endpoint.port()));
    s.append(",");
  }
  s.append(")");
  return s;
}

}}}}
