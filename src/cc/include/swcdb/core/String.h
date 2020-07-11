/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_core_Sring_h
#define swc_core_Sring_h


#include <string>
#include <sstream>


namespace SWC {


  std::string format(const char *fmt, ...)
              __attribute__((format(printf, 1, 2)));


  template <class SequenceT>
  std::string format_list(const SequenceT &seq, const char *sep = ", ") {
    if(seq.empty())
      return "[]";

    auto it = seq.begin();
    auto end = seq.end();

    std::ostringstream out;
    out << '[';
    more:
      out << *it;
      if(++it != end) {
        out << sep;
        goto more;
      }
    out << ']';

    return out.str();
  }


}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/String.cc"
#endif 

#endif 
