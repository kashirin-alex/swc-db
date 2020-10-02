/* 
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/core/Exception.h"
#include "swcdb/core/Serialization.h"
#include "swcdb/core/comm/Event.h"


namespace SWC { namespace Comm {

SWC_SHOULD_INLINE
Event::Ptr Event::make(Type type, int error) {
  return std::make_shared<Event>(type, error);
}

SWC_SHOULD_INLINE
Event::Event(Type type_, int error_) 
            : type(type_), error(error_), expiry_ms(0) {
}

Event::~Event() { }

SWC_SHOULD_INLINE
void Event::received() {
  if(header.timeout_ms)
    expiry_ms = Time::now_ms() + header.timeout_ms; 
}

bool Event::expired(int64_t within) const {
  return expiry_ms && Time::now_ms() > expiry_ms-within;
}

int32_t Event::response_code() {
  if(error || type == Event::ERROR)
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
  out << "Event: type=";
  switch(type){
  case ESTABLISHED:
    out << "ESTABLISHED";
    break;
  case DISCONNECT:
    out << "DISCONNECT";
    break;
  case MESSAGE:
    header.print(out << "MESSAGE ");
    break;
  case ERROR:
    out << "ERROR";
    break;
  default:
    out << "UKNOWN(" << (int)type << ')';
    break;
  }
  if(error) 
    Error::print(out << ' ', error);
    
  /*
  if(data.size) {
    dstr.append(" data=(");
    dstr.append(
      std::string((const char*)data.base, data.size<256?data.size:256));
    dstr.append(")");
  }
  if(data_ext.size) {
    dstr.append(" data_ext=(");
    dstr.append(
      std::string(
        (const char*)data_ext.base, data_ext.size<256?data_ext.size:256));
    dstr.append(")");
  }
  */
}


}} // namespace SWC::Comm

