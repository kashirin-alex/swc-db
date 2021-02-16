/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Compat.h"
#include "swcdb/core/String.h"

#include <cstdarg>
#include <cstdio>


namespace SWC {


std::string format(const char *fmt, ...) {
  std::string res;
  
  std::va_list ap1; 
  std::va_list ap2; 
  va_start(ap1, fmt);
  va_copy(ap2, ap1);
  
  res.resize(std::vsnprintf(nullptr, 0, fmt, ap1));
  std::vsnprintf(res.data(), res.size() + 1, fmt, ap2);
  
  va_end(ap1);
  va_end(ap2);
  return res;
}


} // namespace SWC
