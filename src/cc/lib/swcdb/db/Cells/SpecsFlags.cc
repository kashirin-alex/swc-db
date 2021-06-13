/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Cells/SpecsFlags.h"


namespace SWC { namespace DB { namespace Specs {



std::string Flags::to_string() const {
  std::stringstream ss;
  print(ss);
  return ss.str();
}

void Flags::print(std::ostream& out) const {
  display(out << "Flags(");
  out << ')';
}

void Flags::display(std::ostream& out) const {
  out << "was_set=" << (was_set? "TRUE" : "FALSE");
  if(limit)
    out << " limit=" << limit;
  if(offset)
    out << " offset=" << offset;
  if(max_versions)
     out << " max_versions=" << max_versions;
  if(max_buffer)
    out << " max_buffer=" << max_buffer;
  if(is_only_deletes())
    out << " only_deletes=true";
  if(is_only_keys())
    out << " only_keys=true";
}


}}}
