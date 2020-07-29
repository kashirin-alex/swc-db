/* 
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_core_comm_Event_h
#define swc_core_comm_Event_h

#include "swcdb/core/comm/CommHeader.h"

#include "swcdb/core/Time.h"
#include "swcdb/core/Buffer.h"

#include <memory>

namespace SWC {

class Event final {

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

  ~Event();

  void received();

  bool expired(int64_t within=0) const;
  
  int32_t response_code();

  std::string to_str() const;

  void display();

  Type                type;
  int                 error;
  int64_t             expiry_ms;
  StaticBuffer        data;     //!< Primary data buffer
  StaticBuffer        data_ext; //!< Extended buffer
  CommHeader          header;
};

} // namespace SWC


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/Event.cc"
#endif 

#endif // swc_core_comm_Event_h
