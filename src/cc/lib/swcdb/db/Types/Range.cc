
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Types/Range.h"


namespace SWC { namespace DB { namespace Types {


namespace {
  const char Range_MASTER[]  = "MASTER";
  const char Range_META[]    = "META";
  const char Range_DATA[]    = "DATA";
  const char Range_UNKNOWN[] = "UNKNOWN";
}


std::string to_string(Range typ) {
  switch(typ) {
    case Range::MASTER:
      return Range_MASTER;
    case Range::META:
      return Range_META;
    case Range::DATA:
      return Range_DATA;
    default:
      return Range_UNKNOWN;
  }
}


}}}
