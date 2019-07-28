/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_HostEndPoints_h
#define swc_db_protocol_params_HostEndPoints_h


#include "swcdb/lib/core/Serializable.h"

namespace SWC {
namespace Protocol {
namespace Params {

class HostEndPoints: public Serializable {
  public:

  HostEndPoints() {}

  HostEndPoints(EndPoints points) : endpoints(points) { }
  
  virtual ~HostEndPoints(){ }

  uint8_t encoding_version() const {
    return 1;
  }

  size_t encoded_length_internal() const {
    size_t len = 4;
    for(auto endpoint : endpoints)
      len += Serialization::encoded_length(endpoint);
    return len;
  }

  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_i32(bufp, endpoints.size());
    for(auto endpoint : endpoints)
      Serialization::encode(endpoint, bufp);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp, size_t *remainp) {
    size_t len = Serialization::decode_i32(bufp, remainp);
    for(size_t i=0;i<len;i++)
      endpoints.push_back(Serialization::decode(bufp, remainp));
  }

  std::string to_string() const {
    std::string s("endpoints=(");
    for(auto e : endpoints){
      s.append("[");
      s.append(e.address().to_string());
      s.append("]:");
      s.append(std::to_string(e.port()));
      s.append(",");
    }
    s.append(")");
    return s;
  }
  EndPoints    endpoints;
};
}}}

#endif // swc_db_protocol_params_HostEndPoints_h

