/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/comm/Header.h"


namespace SWC { namespace Comm {


void Header::print(std::ostream& out) const {
  out << "version="     << int(version)
      << " header_len=" << int(header_len)
      << " flags="      << int(flags)
      << " id="         << int(id)
      << " timeout_ms=" << int(timeout_ms)
      << " command="    << int(command)
      << " buffers="    << int(buffers);
  if(buffers) {
    data.print(out);
    data_ext.print(out);
  }
  out << " checksum="   << int(checksum);
}


}}
