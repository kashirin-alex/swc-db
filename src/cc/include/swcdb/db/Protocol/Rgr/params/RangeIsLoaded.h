
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_params_RangeIsLoaded_h
#define swcdb_db_protocol_rgr_params_RangeIsLoaded_h

#include "swcdb/db/Protocol/Common/params/ColRangeId.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Params {

class RangeIsLoaded : public Common::Params::ColRangeId {
  public:

  RangeIsLoaded() {}
  RangeIsLoaded(cid_t cid, rid_t rid) 
                : Common::Params::ColRangeId(cid, rid){}           
  virtual ~RangeIsLoaded() {}

};
  

}}}}}

#endif // swcdb_db_protocol_rgr_params_RangeIsLoaded_h
