
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Types/Range.h"


namespace SWC { namespace Types { 


const std::string to_string(Range typ) {
  switch(typ){
    case Range::MASTER:
      return std::string("MASTER");
    case Range::META:
      return std::string("META");
    case Range::DATA:
      return std::string("DATA");
    default:
      return std::string("uknown");
  }
}


}}
