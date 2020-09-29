/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_db_protocol_common_params_HostEndPoints_h
#define swc_db_protocol_common_params_HostEndPoints_h


#include "swcdb/core/Serializable.h"
#include "swcdb/core/comm/Resolver.h"

namespace SWC { namespace Protocol { namespace Common { namespace Params {
 

class HostEndPoints: public Serializable {
  public:

  HostEndPoints();

  HostEndPoints(const Comm::EndPoints& points);
  
  virtual ~HostEndPoints();

  void set(const Comm::EndPoints& points);

  void print(std::ostream& out) const;

  Comm::EndPoints    endpoints;


  protected:

  size_t internal_encoded_length() const;

  void internal_encode(uint8_t** bufp) const;

  void internal_decode(const uint8_t** bufp, size_t* remainp);


};

}}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Common/params/HostEndPoints.cc"
#endif 

#endif // swc_db_protocol_params_HostEndPoints_h

