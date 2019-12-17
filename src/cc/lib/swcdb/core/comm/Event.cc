/* 
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */



#include "swcdb/core/Error.h"
#include "swcdb/core/Serialization.h"

#include "swcdb/core/comm/Event.h"

namespace SWC {

Event::Ptr Event::make(Type type, int error) {
  return std::make_shared<Event>(type, error);
}

Event::Event(Type type_, int error_) 
            : type(type_), error(error_), expiry_ms(0) {
}

Event::~Event() { }

void Event::received() {
  if(header.timeout_ms)
    expiry_ms = Time::now_ms() + header.timeout_ms; 
}

const bool Event::expired() const {
  return expiry_ms && Time::now_ms() > expiry_ms;
}

int32_t Event::response_code() {
  if (type == Event::ERROR)
    return error;

  const uint8_t *msg = data.base;
  size_t remaining = data.size;

  try { return Serialization::decode_i32(&msg, &remaining); }
  catch (Exception &e) { return e.code(); }
}

const std::string Event::to_str() const {
  std::string dstr("Event: type=");
  switch(type){
  case ESTABLISHED:
    dstr += "ESTABLISHED";
    break;
  case DISCONNECT:
    dstr += "DISCONNECT";
    break;
  case MESSAGE:
    dstr += "MESSAGE " + header.to_string();
    break;
  case ERROR:
    dstr += "ERROR";
    break;
  default:
    dstr += "UKNOWN("+ std::to_string((int)type)+")";
    break;
  }
  if (error != Error::OK){
    dstr.append(" err=");
    dstr.append(std::to_string(error));
    dstr.append("(");
    dstr.append(Error::get_text(error));
    dstr.append(")");
  }
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
  return dstr;
}

void Event::display() {
  SWC_LOG(LOG_INFO, to_str()); 
}

} // namespace SWC

