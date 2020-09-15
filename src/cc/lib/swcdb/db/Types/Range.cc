
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Types/Range.h"


namespace SWC { namespace Types { 


std::string to_string(Range typ) {
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
