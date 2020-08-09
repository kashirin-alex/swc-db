
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Types/MngrRangeState.h"


namespace SWC { namespace Types { 


std::string to_string(MngrRange::State state) {
  switch(state) {

    case MngrRange::State::NOTSET:
      return std::string("NOTSET");

    case MngrRange::State::DELETED:
      return std::string("DELETED");

    case MngrRange::State::ASSIGNED:
      return std::string("ASSIGNED");

    case MngrRange::State::CREATED:
      return std::string("CREATED");

    case MngrRange::State::QUEUED:
      return std::string("QUEUED");
      
    default:
      return std::string("uknown");
  }
}


}}
