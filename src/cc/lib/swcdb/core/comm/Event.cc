/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/core/comm/Event.h"


namespace SWC { namespace Comm {



Event::~Event() noexcept { }

int32_t Event::response_code() const noexcept {
  if(error)
    return error;

  const uint8_t *ptr = data.base;
  size_t remaining = data.size;

  try {
    return Serialization::decode_i32(&ptr, &remaining);
    /* opt
    std::string msg = Serialization::decode_bytes_string(&ptr, &remaining);
    */
  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    return e.code();
  }
}

void Event::print(std::ostream& out) const {
  out <<  "Event: ";
  if(error) {
    Error::print(out << "ERROR ", error);
  } else {
    header.print(out << "MESSAGE ");
    out << " buffers-sz(" << data.size << ',' << data_ext.size << ')';
  }
}


}} // namespace SWC::Comm

