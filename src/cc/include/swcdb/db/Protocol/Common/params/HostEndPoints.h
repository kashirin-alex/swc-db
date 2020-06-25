/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_protocol_common_params_HostEndPoints_h
#define swc_db_protocol_common_params_HostEndPoints_h


#include "swcdb/core/Serializable.h"
#include "swcdb/core/comm/Resolver.h"

namespace SWC { namespace Protocol { namespace Common { namespace Params {
 

class HostEndPoints: public Serializable {
  public:

  HostEndPoints();

  HostEndPoints(const EndPoints& points);
  
  virtual ~HostEndPoints();

  void set(const EndPoints& points);

  std::string to_string() const;

  EndPoints    endpoints;


  protected:

  size_t encoded_length_internal() const;

  void encode_internal(uint8_t** bufp) const;

  void decode_internal(const uint8_t** bufp, size_t* remainp);


};

}}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Common/params/HostEndPoints.cc"
#endif 

#endif // swc_db_protocol_params_HostEndPoints_h

