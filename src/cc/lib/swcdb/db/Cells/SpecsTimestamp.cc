/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/SpecsTimestamp.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {



void Timestamp::display(std::ostream& out) const {
  out << Condition::to_string(comp) << " \"" << value << "\"";
}

void Timestamp::print(std::ostream& out) const {
  out << "Timestamp(";
  if(comp != Condition::NONE)
    out << Condition::to_string(comp) << value;
  out << ')';
}



}}}
