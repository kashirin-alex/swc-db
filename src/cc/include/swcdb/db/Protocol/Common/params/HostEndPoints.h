/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_common_params_HostEndPoints_h
#define swc_db_protocol_common_params_HostEndPoints_h


#include "swcdb/core/Serializable.h"

namespace SWC { namespace Protocol { namespace Common { namespace Params {
 

class HostEndPoints: public Serializable {
  public:

  HostEndPoints() {}

  HostEndPoints(const EndPoints& points) 
               : endpoints(points) { }
  
  virtual ~HostEndPoints(){ }

  void set(const EndPoints& points){
    endpoints.clear();
    endpoints.assign(points.begin(), points.end());
  }

  uint8_t encoding_version() const {
    return 1;
  }

  size_t encoded_length_internal() const {
    size_t len = Serialization::encoded_length_vi32(endpoints.size());
    for(auto& endpoint : endpoints)
      len += Serialization::encoded_length(endpoint);
    return len;
  }

  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi32(bufp, endpoints.size());
    for(auto& endpoint : endpoints)
      Serialization::encode(endpoint, bufp);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp, size_t *remainp) {
    size_t len = Serialization::decode_vi32(bufp, remainp);
    endpoints.clear();
    for(size_t i=0;i<len;i++)
      endpoints.push_back(Serialization::decode(bufp, remainp));
  }

  std::string to_string() const {
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
  
  EndPoints    endpoints;
};

}}}}

#endif // swc_db_protocol_params_HostEndPoints_h

