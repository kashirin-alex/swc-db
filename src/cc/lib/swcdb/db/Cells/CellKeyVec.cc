/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/db/Cells/CellKeyVec.h"


namespace SWC { namespace DB { namespace Cell {



std::string KeyVec::to_string() const {
  std::stringstream ss;
  print(ss);
  return ss.str();
}

void KeyVec::print(std::ostream& out) const {
  out << "Key(";
  if(!size()) {
    out << ')';
    return;
  }
  out << "sz=" << size() << " [";
  char hex[5];
  hex[4] = 0;
  for(auto it = cbegin(); it != cend(); ) {
    out << '"';
    for(auto chrp = it->cbegin(); chrp != it->cend(); ++chrp) {
      if(*chrp == '"')
        out << '\\';
      if(31 < *chrp && *chrp < 127) {
        out << *chrp;
      } else {
        sprintf(hex, "0x%X", *chrp);
        out << hex;
      }
    }
    out << '"';
    if(++it != cend())
      out << ',';
  }
  out << "])";
}



}}}
