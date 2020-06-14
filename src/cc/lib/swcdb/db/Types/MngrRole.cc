
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#include "swcdb/db/Types/MngrRole.h"

namespace SWC { namespace Types { namespace MngrRole {

std::string to_string(uint8_t role) {
  std::string s;

  if(role & SCHEMAS) {
    s.append("SCHEMAS");
  }

  if(role & RANGERS) {
    if(!s.empty())
      s += ',';
    s.append("RANGERS");
  }

  if(role & COLUMNS) {
    if(!s.empty())
      s += ',';
    s.append("COLUMNS");
  }

  if(role & NO_COLUMNS) {
    if(!s.empty())
      s += ',';
    s.append("NO_COLUMNS");
  }
  
  return s;
}

}}}