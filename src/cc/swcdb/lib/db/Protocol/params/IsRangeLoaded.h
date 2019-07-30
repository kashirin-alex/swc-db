
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_IsRangeLoaded_h
#define swc_db_protocol_params_IsRangeLoaded_h

#include "ColRangeId.h"

namespace SWC {
namespace Protocol {
namespace Params {

class IsRangeLoaded : public ColRangeId {
  public:

  IsRangeLoaded() {}
  IsRangeLoaded(size_t cid, size_t rid) 
                : ColRangeId(cid, rid){}           
  virtual ~IsRangeLoaded() {}

};
  

}}}

#endif // swc_db_protocol_params_IsRangeLoaded_h
