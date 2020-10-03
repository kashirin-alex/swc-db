/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_common_params_HostEndPoints_h
#define swcdb_db_protocol_common_params_HostEndPoints_h


#include "swcdb/core/comm/Serializable.h"
#include "swcdb/core/comm/Resolver.h"

namespace SWC { namespace Comm {


//! The SWC-DB Communications Protocol C++ namespace 'SWC::Comm::Protocol'
namespace Protocol { 



/**
 * @brief The SWC-DB Common Communications Protocol C++ namespace 'SWC::Comm::Protocol::Common'
 *
 * \ingroup Database
 */
namespace Common {


namespace Params {
 

class HostEndPoints: public Serializable {
  public:

  HostEndPoints();

  HostEndPoints(const EndPoints& points);
  
  virtual ~HostEndPoints();

  void set(const EndPoints& points);

  void print(std::ostream& out) const;

  EndPoints    endpoints;


  protected:

  size_t internal_encoded_length() const;

  void internal_encode(uint8_t** bufp) const;

  void internal_decode(const uint8_t** bufp, size_t* remainp);


};

}}}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Common/params/HostEndPoints.cc"
#endif 

#endif // swcdb_db_protocol_params_HostEndPoints_h

