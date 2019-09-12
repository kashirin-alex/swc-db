
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_RangeIsLoaded_h
#define swc_db_protocol_params_RangeIsLoaded_h

#include "ColRangeId.h"

namespace SWC {
namespace Protocol {
namespace Params {

class RangeIsLoaded : public ColRangeId {
  public:

  RangeIsLoaded() {}
  RangeIsLoaded(size_t cid, size_t rid) 
                : ColRangeId(cid, rid){}           
  virtual ~RangeIsLoaded() {}

};
  

}}}

#endif // swc_db_protocol_params_RangeIsLoaded_h
