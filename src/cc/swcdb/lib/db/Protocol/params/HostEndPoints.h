/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_HostEndPoints_h
#define swc_db_protocol_params_HostEndPoints_h


#include "swcdb/lib/core/Serializable.h"

namespace SWC {
namespace Protocol {
namespace Params {

struct HostEndPoints {
  public:

  HostEndPoints() {}

  HostEndPoints(EndPoints points) : endpoints(points) { }
  
  virtual ~HostEndPoints(){ }

  size_t encoded_length(){
    size_t len = 4;
    for(auto endpoint : endpoints)
      len += Serialization::encoded_length(endpoint);
    return len;
  }

  void encode(uint8_t **bufp){
    Serialization::encode_i32(bufp, endpoints.size());
    for(auto endpoint : endpoints)
      Serialization::encode(endpoint, bufp);
  }

  void decode(const uint8_t **bufp, size_t *remainp) {
    size_t len = Serialization::decode_i32(bufp, remainp);
    for(size_t i=0;i<len;i++)
      endpoints.push_back(Serialization::decode(bufp, remainp));
  }

  std::string to_string(){
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

