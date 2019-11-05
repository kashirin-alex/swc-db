/* 
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_comm_Event_h
#define swc_core_comm_Event_h

#include "CommHeader.h"

#include "../Clock.h"
#include "../Error.h"
#include "../StaticBuffer.h"

#include <iostream>
#include <memory>

namespace SWC {

class Event {

  public:

  /** Enumeration for event types.*/
  enum Type { 
    ESTABLISHED,  ///< Connection established event
    DISCONNECT,   ///< Connection disconnected event
    MESSAGE,      ///< Request/response message event
    ERROR,        ///< %Error event
  };

  typedef std::shared_ptr<Event> Ptr;

  static Ptr make(Type type, int error) {
    return std::make_shared<Event>(type, error);
  }

  explicit Event(Type type_, int error_) 
                : type(type_), error(error_), 
                  arrival_time(ClockT::now()) {
   }

  ~Event() { }

  /** Deadline for request.
   * @return Absolute deadline
   */
  ClockT::time_point deadline() {
    HT_ASSERT(arrival_time.time_since_epoch().count() > 0);
    return arrival_time + std::chrono::milliseconds(header.timeout_ms);
  }

  /** Generates a one-line string representation of the event.  For example:
   * <pre>
   *   Event: type=MESSAGE id=2 gid=0 header_len=16 total_len=20 \
   *   from=127.0.0.1:15861 ...
   * </pre>
   */
  const std::string to_str() const {
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
      dstr.append(std::string((const char*)data.base, data.size<256?data.size:256));
      dstr.append(")");
    }
    if(data_ext.size) {
      dstr.append(" data_ext=(");
      dstr.append(std::string((const char*)data_ext.base, data_ext.size<256?data_ext.size:256));
      dstr.append(")");
    }
    return dstr;
  }

  /** Displays a one-line string representation of the event to stdout.
   * @see to_str
   */
  void display() {
    std::cerr << to_str() << std::endl; 
  }
  

  Type                type;
  int                 error;
  ClockT::time_point  arrival_time;
  StaticBuffer        data;     //!< Primary data buffer
  StaticBuffer        data_ext; //!< Extended buffer
  CommHeader          header;
};

} // namespace SWC

#endif // swc_core_comm_Event_h
