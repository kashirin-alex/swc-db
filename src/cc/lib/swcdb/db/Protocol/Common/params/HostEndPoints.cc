/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
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

uint8_t HostEndPoints::encoding_version() const {
  return 1;
}

size_t HostEndPoints::encoded_length_internal() const {
  size_t len = Serialization::encoded_length_vi32(endpoints.size());
  for(auto& endpoint : endpoints)
    len += Serialization::encoded_length(endpoint);
  return len;
}

void HostEndPoints::encode_internal(uint8_t **bufp) const {
  Serialization::encode_vi32(bufp, endpoints.size());
  for(auto& endpoint : endpoints)
    Serialization::encode(endpoint, bufp);
}

void HostEndPoints::decode_internal(uint8_t version, const uint8_t **bufp,
                                    size_t *remainp) {
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
