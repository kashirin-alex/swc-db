/* 
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_comm_Event_h
#define swc_core_comm_Event_h

#include "swcdb/core/comm/CommHeader.h"

#include "swcdb/core/Time.h"
#include "swcdb/core/StaticBuffer.h"

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

  static Ptr make(Type type, int error);

  explicit Event(Type type_, int error_);

  virtual ~Event();

  ClockT::time_point deadline();
  
  int32_t response_code();

  const std::string to_str() const;

  void display();

  Type                type;
  int                 error;
  ClockT::time_point  arrival_time;
  StaticBuffer        data;     //!< Primary data buffer
  StaticBuffer        data_ext; //!< Extended buffer
  CommHeader          header;
};

} // namespace SWC


#ifdef SWC_IMPL_SOURCE
#include "../../../../lib/swcdb/core/comm/Event.cc"
#endif 

#endif // swc_core_comm_Event_h
