/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Common/params/HostEndPoints.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace Protocol { namespace Common { namespace Params {
 
HostEndPoints::HostEndPoints() {}

HostEndPoints::HostEndPoints(const Comm::EndPoints& points) 
                            : endpoints(points) { }

HostEndPoints::~HostEndPoints() { }

void HostEndPoints::set(const Comm::EndPoints& points) {
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

void HostEndPoints::print(std::ostream& out) const {
  out << "endpoints=[";
  for(auto& endpoint : endpoints)
    out << endpoint << ',';
  out << ']';
}

}}}}
