/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/db/Types/MngrRole.h"

namespace SWC { namespace DB { namespace Types { namespace MngrRole {


namespace {
  const char MngrRole_SCHEMAS[]     = "SCHEMAS";
  const char MngrRole_RANGERS[]     = "RANGERS";
  const char MngrRole_COLUMNS[]     = "COLUMNS";
  const char MngrRole_NO_COLUMNS[]  = "NO_COLUMNS";
}


std::string to_string(uint8_t role) {
  std::string s;

  if(role & SCHEMAS) {
    s.append(MngrRole_SCHEMAS);
  }

  if(role & RANGERS) {
    if(!s.empty())
      s.append(",");
    s.append(MngrRole_RANGERS);
  }

  if(role & COLUMNS) {
    if(!s.empty())
      s.append(",");
    s.append(MngrRole_COLUMNS);
  }

  if(role & NO_COLUMNS) {
    if(!s.empty())
      s.append(",");
    s.append(MngrRole_NO_COLUMNS);
  }

  return s;
}

}}}}
